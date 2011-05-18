#include <geekos/int.h>
#include <geekos/malloc.h>
#include <geekos/string.h>
#include <geekos/kthread.h>
#include <geekos/sem.h>
#include <geekos/bitset.h>


#ifdef DEBUG_SEM

#define DEBUG_PRINT(ftm,...) do{ Print(ftm, ## __VA_ARGS__ ); } while( false )
#else
#define DEBUG_PRINT(ftm, ...) do{ } while ( false )

#endif

struct Semaphore{
    struct Thread_Queue waitQueue;
    int count;
    int ival;
    char *name;
}s_AllSemaphoreList[MAX_SEM];

static tlocal_key_t key = -1;

static void Sem_Destructor(void *bitmap)
{
    int sem = 0;

    if (bitmap != NULL) {
        for (sem = 0; sem < MAX_SEM; sem++) {
            if ( Is_Bit_Set(bitmap, sem) == 0) {
                Destroy_Semaphore(sem);
            }
        }

        Destroy_Bit_Set(bitmap);
        Tlocal_Put(key, NULL);
        bitmap = NULL;
    }
}

static int Check_Semaphore(int sem){

    void *bitmap = NULL;
    int retVal = 0;

    if( sem < 0 || sem >= MAX_SEM ){
        DEBUG_PRINT("FATAL: sem id out of range\n");
        retVal = -1;
        goto fail;
    };

    if (key == -1){
        DEBUG_PRINT("FATAL: key %i not created\n",key);
        retVal = -1;
        goto fail;
    };

    bitmap = Tlocal_Get(key);

    if (bitmap == NULL){
        DEBUG_PRINT("FATAL: TDS not exist , key %i - ",key);
        retVal = -1;
        goto fail;
    }

    if(!Is_Bit_Set(bitmap,sem)){
        DEBUG_PRINT("%s not in %p pid: %i\n",s_AllSemaphoreList[sem].name,bitmap, g_currentThread->pid);
        retVal = -1;
    }

fail:
    return retVal;
}


int Create_Semaphore(char *name, int ival)
{
    int i = 0, idSem = -1, lName=0;
    bool newSem = true;
    void *bitmap = NULL;

    KASSERT(name != NULL);
    KASSERT(ival >= 0);
    lName = strlen(name);
    KASSERT(lName <= MAX_NAME_SEM);

    bool flags = Begin_Int_Atomic();

    for(i=0;i<MAX_SEM ;i++){
        if(s_AllSemaphoreList[i].name != NULL){
            if(strncmp(s_AllSemaphoreList[i].name, name,lName) == 0){
                idSem = i;
                s_AllSemaphoreList[i].count++;
                newSem = false;
                break;
            }
        }else{
            idSem = i;
        }
    }

    if(newSem){
        s_AllSemaphoreList[idSem].name = name;
        s_AllSemaphoreList[idSem].count = 1;
        s_AllSemaphoreList[idSem].ival = ival;
    }

    if (key == -1){
        bitmap = Create_Bit_Set(MAX_SEM);
        if( (Tlocal_Create(&key,Sem_Destructor) != 0) ){
            DEBUG_PRINT("FATAL: Tlocal_Create fail\n");
            return -1;
        }
        DEBUG_PRINT("INFO: pid %i: New bitmap at %p\n",g_currentThread->pid,bitmap);
    }else{
        bitmap = Tlocal_Get(key); 
        if (bitmap == NULL){
             bitmap = Create_Bit_Set(MAX_SEM);
             DEBUG_PRINT("INFO: pid %i: New bitmap at %p\n",g_currentThread->pid,bitmap);
         }
    }

    if ( (idSem != -1) && (key != -1) && (bitmap != NULL)){
        Set_Bit(bitmap, idSem);
        Tlocal_Put(key, bitmap);
        DEBUG_PRINT("INFO pid %i: Put %s in bitmap %p\n",g_currentThread->pid,s_AllSemaphoreList[idSem].name,bitmap);
    }else{
        DEBUG_PRINT("Error: al Create Sem: %s %i %p \n",s_AllSemaphoreList[idSem].name,key,bitmap);
        return -1;
    }

    End_Int_Atomic(flags);
    return idSem;
}

int P(int sem)
{
    if (Check_Semaphore(sem) != 0) {
        DEBUG_PRINT("P %i\n",sem);
        return -1;
    } else {
        bool iflag = Begin_Int_Atomic();
        s_AllSemaphoreList[sem].ival--;
        DEBUG_PRINT("INFO: %i P(%s): ival = %i\n",g_currentThread->pid,s_AllSemaphoreList[sem].name,s_AllSemaphoreList[sem].ival);
        if (s_AllSemaphoreList[sem].ival < 0){
            Wait(&s_AllSemaphoreList[sem].waitQueue);
        }
        End_Int_Atomic(iflag);
    }
    return 0;
}

int V(int sem)
{

   if (Check_Semaphore(sem) != 0) {
       DEBUG_PRINT("V %i\n",sem);
       return -1;
    } else {
        bool iflag = Begin_Int_Atomic();
        s_AllSemaphoreList[sem].ival++;
        DEBUG_PRINT("INFO: %i V(%s): ival = %i\n",g_currentThread->pid,s_AllSemaphoreList[sem].name,s_AllSemaphoreList[sem].ival);
        if (s_AllSemaphoreList[sem].ival <= 0) {
            Wake_Up(&s_AllSemaphoreList[sem].waitQueue);
        }
        End_Int_Atomic(iflag);
    }

    return 0;
}

int Destroy_Semaphore(int sem)
{
    if (Check_Semaphore(sem) != 0) {
        DEBUG_PRINT("Destroy_Semaphore %i\n",sem);
        return -1;
    } else {
        bool iflag = Begin_Int_Atomic();
        void *bitmap = Tlocal_Get(key);
        Clear_Bit(bitmap,sem);
        s_AllSemaphoreList[sem].count--;
        if (s_AllSemaphoreList[sem].count == 0) {
            Free(s_AllSemaphoreList[sem].name);
            s_AllSemaphoreList[sem].name = NULL;
        }
        End_Int_Atomic(iflag);
    }
    return 0;
}

