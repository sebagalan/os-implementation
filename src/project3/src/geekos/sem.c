#include <geekos/sem.h>
#include <geekos/ktypes.h>
#include <geekos/string.h>
#include <geekos/screen.h>
#include <geekos/malloc.h>
#include <geekos/kthread.h>
#include <geekos/user.h>
#include <geekos/errno.h>
#include <geekos/bitset.h>
#include <geekos/int.h>

#ifdef DEBUG_SEM
#define DEBUG_PRINT(ftm,...) do{ Print(ftm, ## __VA_ARGS__ ); } while( false )
#else
#define DEBUG_PRINT(ftm, ...) do{ } while ( false )
#endif
/* Los semáforos */
struct Semaphore g_Semaphores[MAX_NUM_SEMAPHORES];

/* Funciones auxiliares */
static int isSemaphoreCreated(char *namesem, int nameLength);
static int getFreeSemaphore();
/* static bool hasAccess(int sid); */
static bool validateSID(int sid);
/*Liberar todos los semaforos que adquiere el kthread*/
static void freeSemaphoresInBitmap(void *bitmap);

/*llave para Thread local data*/
static tlocal_key_t key = -1;

int CreateSemaphore(char *name, int nameLength, int initCount)
{
    int sid = 0;
    int ret = -1;
    void *bitmap = NULL;

    /* Paranoico, soy entrypoint de userspace */
    if (name==NULL || nameLength<=0 || initCount<0 ||
        MAX_SEMAPHORE_NAME<nameLength ||
        strnlen(name, MAX_SEMAPHORE_NAME)!=nameLength) {
            return EINVALID;
    }

    bool atomic = Begin_Int_Atomic();
    sid = isSemaphoreCreated(name, nameLength);

    /* sid es negativo si no fue creado, de lo contrario
     * sid es el ID del semaforo ya creado
     */
    if (sid >= 0) {
        /* Semaforo ya creado */
        Free(name);
        g_Semaphores[sid].refCounter++;
        ret = sid;
    }

    if (sid < 0) {
        /* Semaforo no creado. Crear */
        sid = getFreeSemaphore();
        if (sid < 0) 
            /* No hay semaforos disponibles nos vamos rapido */
            return -1;

        memset(g_Semaphores[sid].name,'\0',MAX_NUM_SEMAPHORES+1);
        strncpy(g_Semaphores[sid].name, name, nameLength);
        g_Semaphores[sid].available = true;
        g_Semaphores[sid].resourcesCount = initCount;
        g_Semaphores[sid].refCounter = 1;
        ret = sid;
    }

    if (key == -1) {
        bitmap = Create_Bit_Set(MAX_NUM_SEMAPHORES);
        if ( (Tlocal_Create(&key,freeSemaphoresInBitmap) != 0) ) {
            DEBUG_PRINT("FATAL: Tlocal_Create fail\n");
            ret = -1;
        }
        DEBUG_PRINT("INFO: pid %i: New bitmap at %p\n",g_currentThread->pid,bitmap);
    } else {
        bitmap = Tlocal_Get(key); 
        if (bitmap == NULL) {
             bitmap = Create_Bit_Set(MAX_NUM_SEMAPHORES);
             DEBUG_PRINT("INFO: pid %i: New bitmap at %p\n",g_currentThread->pid,bitmap);
         }
    }

    if( (sid != -1) && (key != -1) && (bitmap != NULL) ){
        Set_Bit(bitmap, sid);
        Tlocal_Put(key, bitmap);
    }

    End_Int_Atomic(atomic);
    return ret;
}

int V(int sid)
{
    if (!validateSID(sid)) {
        return EINVALID;
    }

    bool atomic = Begin_Int_Atomic();
    g_Semaphores[sid].resourcesCount++;
    DEBUG_PRINT("INFO: %i V(%s): ival = %i\n",g_currentThread->pid,g_Semaphores[sid].name,g_Semaphores[sid].resourcesCount);
    if (g_Semaphores[sid].resourcesCount <= 0) {
        Wake_Up(&g_Semaphores[sid].waitingThreads);
    }
    End_Int_Atomic(atomic);
    return 0;
}

int P(int sid)
{
    /*KASSERT(g_Semaphores[sid].available == true);*/
    if (!validateSID(sid)) {
        return EINVALID;
    }

    bool atomic = Begin_Int_Atomic();
    g_Semaphores[sid].resourcesCount--;
    DEBUG_PRINT("INFO: %i P(%s): ival = %i\n",g_currentThread->pid,g_Semaphores[sid].name,g_Semaphores[sid].resourcesCount);
    if (g_Semaphores[sid].resourcesCount < 0) {
        Wait(&g_Semaphores[sid].waitingThreads);
    }
    End_Int_Atomic(atomic);

    return 0;
}

int DestroySemaphore(int sid){

    if (!validateSID(sid)) {
        return EINVALID;
    }

    bool atomic = Begin_Int_Atomic();

    void *bitmap = Tlocal_Get(key);
    Clear_Bit(bitmap,sid);

    g_Semaphores[sid].refCounter--;
    if (g_Semaphores[sid].refCounter == 0){
        g_Semaphores[sid].available = false;
        memset(&g_Semaphores[sid].name,'\0',MAX_SEMAPHORE_NAME+1);
    }

    End_Int_Atomic(atomic);

    return 0;
}


/* --- Funciones auxiliares --- */

static bool validateSID(int sid)
{
    bool retVal = true;
    void *bitmap = NULL;

    if (sid < 0 ||
        sid >= MAX_NUM_SEMAPHORES ||
        !g_Semaphores[sid].available) {
        retVal = false;
        goto end;
    }

    if (key == -1) {
        DEBUG_PRINT("FATAL: key %i not created\n",key);
        retVal = false;
        goto end;
    }

    bitmap = Tlocal_Get(key);

    if (bitmap == NULL) {
        DEBUG_PRINT("FATAL: TDS not exist , key %i, pid %i\n",key,g_currentThread->pid);
        retVal = false;
        goto end;
    }

    if(!Is_Bit_Set(bitmap,sid)){
        DEBUG_PRINT("%s not in %p pid: %i\n",g_Semaphores[sid].name,bitmap, g_currentThread->pid);
        retVal = false;
    }

end:
    return retVal; 
}

static int isSemaphoreCreated(char *namesem, int lengthName)
{
    int sid = 0;
    int ret = -1;

    for (sid = 0; sid < MAX_NUM_SEMAPHORES; sid++) {
        if (strncmp(g_Semaphores[sid].name, namesem, lengthName) == 0) {
            ret = sid;
            break;
        }
    }

    return ret;
}

static int getFreeSemaphore()
{
    int sid = 0;
    int ret = -1;

    for (sid = 0; sid < MAX_NUM_SEMAPHORES; sid++) {
        if (!g_Semaphores[sid].available) {
            ret = sid;
            break;
        }
    }

    return ret;
}

static void freeSemaphoresInBitmap(void *bitmap)
{
    int sem = 0;

    if (bitmap != NULL) {
        for (sem = 0; sem < MAX_NUM_SEMAPHORES; sem++) {
            if ( Is_Bit_Set(bitmap, sem) == 0) {
                DestroySemaphore(sem);
            }
        }

        Destroy_Bit_Set(bitmap);
        Tlocal_Put(key, NULL);
        bitmap = NULL;
    }
}
