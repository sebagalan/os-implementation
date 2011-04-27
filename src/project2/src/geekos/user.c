/*
 * Common user mode functions
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.50 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/errno.h>
#include <geekos/ktypes.h>
#include <geekos/kassert.h>
#include <geekos/int.h>
#include <geekos/mem.h>
#include <geekos/malloc.h>
#include <geekos/kthread.h>
#include <geekos/vfs.h>
#include <geekos/tss.h>
#include <geekos/user.h>

#ifdef DEBUG
#define DEBUG_PRINT(ftm,...) do{ Print(ftm, ## __VA_ARGS__ ); } while( false )
#else
#define DEBUG_PRINT(ftm, ...) do{ } while ( false )
#endif

/*
 * This module contains common functions for implementation of user
 * mode processes.
 */

/*
 * Associate the given user context with a kernel thread.
 * This makes the thread a user process.
 */
void Attach_User_Context(struct Kernel_Thread* kthread, struct User_Context* context)
{
    KASSERT(context != 0);
    kthread->userContext = context;

    Disable_Interrupts();

    /*
     * We don't actually allow multiple threads
     * to share a user context (yet)
     */
    KASSERT(context->refCount == 0);

    ++context->refCount;
    Enable_Interrupts();
}

/*
 * If the given thread has a user context, detach it
 * and destroy it.  This is called when a thread is
 * being destroyed.
 */
void Detach_User_Context(struct Kernel_Thread* kthread)
{
    struct User_Context* old = kthread->userContext;

    kthread->userContext = 0;

    if (old != 0) {
	int refCount;

	Disable_Interrupts();
        --old->refCount;
	refCount = old->refCount;
	Enable_Interrupts();

	DEBUG_PRINT("User context refcount == %d\n", refCount);
        if (refCount == 0)
            Destroy_User_Context(old);
    }
}

/*
 * Spawn a user process.
 * Params:
 *   program - the full path of the program executable file
 *   command - the command, including name of program and arguments
 *   pThread - reference to Kernel_Thread pointer where a pointer to
 *     the newly created user mode thread (process) should be
 *     stored
 * Returns:
 *   The process id (pid) of the new process, or an error code
 *   if the process couldn't be created.  Note that this function
 *   should return ENOTFOUND if the reason for failure is that
 *   the executable file doesn't exist.
 */
 
 int Spawn(const char *program, const char *command, struct Kernel_Thread **pThread)
{
  /*
     * Hints:
     * - Call Read_Fully() to load the entire executable into a memory buffer
     * - Call Parse_ELF_Executable() to verify that the executable is
     *   valid, and to populate an Exe_Format data structure describing
     *   how the executable should be loaded
     * - Call Load_User_Program() to create a User_Context with the loaded
     *   program
     * - Call Start_User_Thread() with the new User_Context
     *
     * If all goes well, store the pointer to the new thread in
     * pThread and return 0.  Otherwise, return an error code.
     *
     * 
     * lo mismo que lprog.c en project1? seguramente cambia algo...*/
    
	char *exeFileData = 0;
	ulong_t exeFileLength;
	struct Exe_Format exeFormat;
	struct User_Context *userContext = NULL;
	int result = 0;
	
	result = Read_Fully(program, (void**) &exeFileData, &exeFileLength);
	if (result != 0){
		DEBUG_PRINT("Read_Fully failed to read %s from disk\n", program);
		return result;
	}

	if (Parse_ELF_Executable(exeFileData, exeFileLength, &exeFormat) != 0){
		DEBUG_PRINT("Parse_ELF_Executable failed\n");
		Free(exeFileData);
		return -1;
	}

	if(Load_User_Program(exeFileData, exeFileLength, &exeFormat, command, &userContext) != 0){
		DEBUG_PRINT("Load_User_Program failed\n");
		Free(exeFileData);
		return -1;
	}

	/* The kernel thread id; also used as process id */

	(*pThread) = Start_User_Thread(userContext, true);
	
	if (pThread == NULL){
		DEBUG_PRINT("Start_User_Thread failed\n");
		Free(exeFileData);
		Free(userContext);
		return -1; 	
	}else{
		DEBUG_PRINT("Spawn: pid = %i\n",(*pThread)->pid);
	}	
	Free(exeFileData);
	return result;
}


/*
 * If the given thread has a User_Context,
 * switch to its memory space.
 *
 * Params:
 *   kthread - the thread that is about to execute
 *   state - saved processor registers describing the state when
 *      the thread was interrupted
 */

void Switch_To_User_Context(struct Kernel_Thread* kthread, struct Interrupt_State* state)
{
	/*
	* Hint: Before executing in user mode, you will need to call
	* the Set_Kernel_Stack_Pointer() and Switch_To_Address_Space()
	* functions.
	*/
	 
	KASSERT(kthread!=NULL &&  state!=NULL);
		
	if (kthread->userContext!=NULL){
		/*Move the stack pointer up one page
		 * http://www.cs.umd.edu/class/spring2005/cmsc412/proj2/
		 * */
		Set_Kernel_Stack_Pointer(((ulong_t)kthread->stackPage) + PAGE_SIZE);   		
		Switch_To_Address_Space(kthread->userContext);
		/*DEBUG_PRINT("Switch_To_User_Context: kthread = %i\tuserContext = %p\n",
				kthread->pid,kthread->userContext);*/
	}
}

