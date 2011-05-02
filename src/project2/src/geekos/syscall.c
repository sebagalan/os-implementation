/*
 * System call handlers
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2003,2004 David Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.59 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/syscall.h>
#include <geekos/errno.h>
#include <geekos/kthread.h>
#include <geekos/int.h>
#include <geekos/elf.h>
#include <geekos/malloc.h>
#include <geekos/screen.h>
#include <geekos/keyboard.h>
#include <geekos/string.h>
#include <geekos/user.h>
#include <geekos/timer.h>
#include <geekos/vfs.h>


#ifdef DEBUG
#define DEBUG_PRINT(ftm,...) do{ Print(ftm, ## __VA_ARGS__ ); } while( false )
#else
#define DEBUG_PRINT(ftm, ...) do{ } while ( false )
#endif

/*
 * Null system call.
 * Does nothing except immediately return control back
 * to the interrupted user program.
 * Params:
 *  state - processor registers from user mode
 *
 * Returns:
 *   always returns the value 0 (zero)
 */
static int Sys_Null(struct Interrupt_State* state)
{
    Print("Sys_Null was called.\n");
    return 0;
}

/*
 * Exit system call.
 * The interrupted user process is terminated.
 * Params:
 *   state->ebx - process exit code
 * Returns:
 *   Never returns to user mode!
 */
static int Sys_Exit(struct Interrupt_State* state)
{
	/* sti */
	Enable_Interrupts();
	DEBUG_PRINT("Sys_Exit");
	if(g_currentThread->userContext){
		Detach_User_Context(g_currentThread);
	}
	/* cli */
	Disable_Interrupts();	
	Exit(state->ebx);
	return 0;
}

/*
 * Print a string to the console.
 * Params:
 *   state->ebx - user pointer of string to be printed
 *   state->ecx - number of characters to print
 * Returns: 0 if successful, -1 if not
 */
static int Sys_PrintString(struct Interrupt_State* state)
{
  /*  TODO("PrintString system call");*/
    int result = 0;
    char *the_string = NULL;
    unsigned int the_sting_length = (sizeof(char)*state->ecx)+1;

    if(state->ecx < 1024) {
        the_string = (char*)Malloc(the_sting_length);
        memset(the_string,'\0',the_sting_length);
        KASSERT(the_string != NULL);
        if ( Copy_From_User(the_string,state->ebx,state->ecx) ){
            Put_Buf(the_string,state->ecx);
        }else{
            result = -1;
        }
        Free(the_string);
    } else {
        result = -1;
    }
    return result; 
}

/*
 * Get a single key press from the console.
 * Suspends the user process until a key press is available.
 * Params:
 *   state - processor registers from user mode
 * Returns: the key code
 */
static int Sys_GetKey(struct Interrupt_State* state)
{
    /*TODO("GetKey system call");*/
    return Wait_For_Key(); 
}

/*
 * Set the current text attributes.
 * Params:
 *   state->ebx - character attributes to use
 * Returns: always returns 0
 */
static int Sys_SetAttr(struct Interrupt_State* state)
{
    /*TODO("SetAttr system call");*/
    Set_Current_Attr(state->ebx);
    return 0;
}

/*
 * Get the current cursor position.
 * Params:
 *   state->ebx - pointer to user int where row value should be stored
 *   state->ecx - pointer to user int where column value should be stored
 * Returns: 0 if successful, -1 otherwise
 */
static int Sys_GetCursor(struct Interrupt_State* state)
{
    /*TODO("GetCursor system call");*/
    int row = 0, col = 0;
    
    Get_Cursor(&row,&col);
    if (row < 0 || row >= NUMROWS || col < 0 || col >= NUMCOLS) {
        return -1;
    }

    if( !(Copy_To_User(state->ebx, &row,sizeof(int)) && Copy_To_User(state->ecx, &col,sizeof(int))) ) {
        return -1;
    }

    return 0;
}

/*
 * Set the current cursor position.
 * Params:
 *   state->ebx - new row value
 *   state->ecx - new column value
 * Returns: 0 if successful, -1 otherwise
 */
static int Sys_PutCursor(struct Interrupt_State* state)
{
   /* TODO("PutCursor system call");*/
    if(!Put_Cursor(state->ebx,state->ecx)){
        return -1;
    }
    return 0;
}

/*
 * Create a new user process.
 * Params:
 *   state->ebx - user address of name of executable
 *   state->ecx - length of executable name
 *   state->edx - user address of command string
 *   state->esi - length of command string
 * Returns: pid of process if successful, error code (< 0) otherwise
 */
static int Sys_Spawn(struct Interrupt_State* state)
{
   /* TODO("Spawn system call");*/
    
    int pid = 0;
    char *program = NULL;
    char *command = NULL;
    unsigned int command_length = 0, program_length = 0;
    struct Kernel_Thread *pThread = NULL;
    
    if (state->ecx < VFS_MAX_PATH_LEN){

        program_length = (sizeof(char)*state->ecx)+1;
        command_length = (sizeof(char)*state->esi)+1;

        program = (char *)Malloc(program_length);
        memset(program,'\0',program_length);
      
        command = (char *)Malloc(command_length);
        memset(command,'\0',command_length);
        
        if(Copy_From_User(program,state->ebx,program_length) &&
           Copy_From_User(command,state->edx,command_length)) {
                DEBUG_PRINT("program = %s\ncommand = %s\n",program,command);
                Enable_Interrupts();/*sti*/
                pid = Spawn(program,command,&pThread);
                Disable_Interrupts();/*cli*/
        } else {
            pid = -1;
        }
        Free(program);
        Free(command);
    }else{
        pid = -1;
    }

   return pid;
}

/*
 * Wait for a process to exit.
 * Params:
 *   state->ebx - pid of process to wait for
 * Returns: the exit code of the process,
 *   or error code (< 0) on error
 */
static int Sys_Wait(struct Interrupt_State* state)
{

/*    TODO("Wait system call");*/
    int exitCode = 0;
    struct Kernel_Thread *result  = NULL;

    result = Lookup_Thread(state->ebx);
    if (result != NULL &&  result->owner == g_currentThread){
        /* sti */
        Enable_Interrupts();
        exitCode = Join(result);
        /* cli */
        Disable_Interrupts();
    }else{
        exitCode = -1;
    }

    return exitCode;
}

/*
 * Get pid (process id) of current thread.
 * Params:
 *   state - processor registers from user mode
 * Returns: the pid of the current thread
 */
static int Sys_GetPID(struct Interrupt_State* state)
{
    /*TODO("GetPID system call");*/
    return g_currentThread->pid;
}


/*
 * Global table of system call handler functions.
 */
const Syscall g_syscallTable[] = {
    Sys_Null,
    Sys_Exit,
    Sys_PrintString,
    Sys_GetKey,
    Sys_SetAttr,
    Sys_GetCursor,
    Sys_PutCursor,
    Sys_Spawn,
    Sys_Wait,
    Sys_GetPID,
};

/*
 * Number of system calls implemented.
 */
const int g_numSyscalls = sizeof(g_syscallTable) / sizeof(Syscall);
