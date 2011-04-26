/*
 * Segmentation-based user mode implementation
 * Copyright (c) 2001,2003 David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.23 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/ktypes.h>
#include <geekos/kassert.h>
#include <geekos/defs.h>
#include <geekos/mem.h>
#include <geekos/string.h>
#include <geekos/malloc.h>
#include <geekos/int.h>
#include <geekos/gdt.h>
#include <geekos/segment.h>
#include <geekos/tss.h>
#include <geekos/kthread.h>
#include <geekos/argblock.h>
#include <geekos/user.h>

/* ----------------------------------------------------------------------
 * Variables
 * ---------------------------------------------------------------------- */

#define DEFAULT_USER_STACK_SIZE 8192

#define LDT_CS  0
#define LDT_DS  1


#ifdef DEBUG
#define DEBUG_PRINT(ftm,...) do{ Print(ftm, ## __VA_ARGS__ ); } while( false )
#else
#define DEBUG_PRINT(ftm, ...) do{ } while ( false )
#endif

/* ----------------------------------------------------------------------
 * Private functions
 * ---------------------------------------------------------------------- */


/*
 * Create a new user context of given size
 */

/* TODO: Implement*/
static struct User_Context* Create_User_Context(ulong_t size){

	struct User_Context *userContext = NULL;

	userContext = (struct User_Context *)Malloc(sizeof(struct User_Context));
	if(userContext != NULL ){
		memset(userContext,0,sizeof(struct User_Context));
		userContext->memory = (char *)Malloc(size);
		
		userContext->size = size;
		/*
		-	create an LDT for the process
		-	add a descriptor to the GDT that describes the location of the LDT
		-	create a selector that contains the location of the LDT descriptor within the GDT
		-	create descriptors for the code and data segments of the user program and add these descriptors to the LDT
		-	create selectors that contain the locations of the two descriptors within the LDT
		-	project1/src/geekos/lgrog.c
		 */
				
		userContext->ldtDescriptor = Allocate_Segment_Descriptor();
		
		Init_LDT_Descriptor(userContext->ldtDescriptor,
					userContext->ldt, NUM_USER_LDT_ENTRIES);
		
		KASSERT(userContext->ldtDescriptor!=NULL);			
		userContext->ldtSelector = Selector( KERNEL_PRIVILEGE, true, 
					Get_Descriptor_Index(userContext->ldtDescriptor));
		

		/*descriptors and selectors for code and data*/

		/*Code segment and selector*/
		
		Init_Code_Segment_Descriptor(&userContext->ldt[LDT_CS],
					   (ulong_t)userContext->memory, /* base address*/
					   (size/PAGE_SIZE)+10,	/* num pages*/
					   USER_PRIVILEGE	/* privilege */
					   );
		userContext->csSelector = Selector(USER_PRIVILEGE,false,LDT_CS);

		/*Data segment and selector*/
		Init_Data_Segment_Descriptor(&userContext->ldt[LDT_DS],
					   (ulong_t)userContext->memory,(size/PAGE_SIZE)+10,
					   USER_PRIVILEGE);
	
		userContext->dsSelector = Selector(USER_PRIVILEGE,false,LDT_DS);
	    userContext->refCount = 0; 
	}

	return userContext;
}

static bool Validate_User_Memory(struct User_Context* userContext,
    ulong_t userAddr, ulong_t bufSize)
{
    ulong_t avail;

    if (userAddr >= userContext->size)
        return false;

    avail = userContext->size - userAddr;
    if (bufSize > avail)
        return false;

    return true;
}


static ulong_t Find_Maximum_Virtual_Address(struct Exe_Format *exeFormat)
{
	struct Exe_Segment *segment = NULL;
	ulong_t topva = 0, maxva = 0;
	unsigned int i = 0;
	
	if (exeFormat != NULL){
		for (i = 0; i < exeFormat->numSegments; ++i){
			segment = &exeFormat->segmentList[i];
			if (segment != NULL){
				topva = segment->startAddress + segment->sizeInMemory; 

				if (topva > maxva){
					maxva = topva;
				}
			}
		}
	}
	
	return maxva;
}

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

/*
 * Destroy a User_Context object, including all memory
 * and other resources allocated within it.
 */
 
void Destroy_User_Context(struct User_Context* userContext)
{
    /*
     * Hints:
     * - you need to free the memory allocated for the user process
     * - don't forget to free the segment descriptor allocated
     *   for the process's LDT
     */
	/*TODO("Destroy a User_Context");*/

	Free_Segment_Descriptor(userContext->ldtDescriptor);
	Free(userContext->memory);
	Free(userContext);
	userContext = NULL;
	DEBUG_PRINT("Destroy a User_Context\n");
}
  

/*
 * Load a user executable into memory by creating a User_Context
 * data structure.
 * Params:
 * exeFileData - a buffer containing the executable to load
 * exeFileLength - number of bytes in exeFileData
 * exeFormat - parsed ELF segment information describing how to
 *   load the executable's text and data segments, and the
 *   code entry point address
 * command - string containing the complete command to be executed:
 *   this should be used to create the argument block for the
 *   process
 * pUserContext - reference to the pointer where the User_Context
 *   should be stored
 *
 * Returns:
 *   0 if successful, or an error code (< 0) if unsuccessful
 */

int Load_User_Program(char *exeFileData, ulong_t exeFileLength,
    struct Exe_Format *exeFormat, const char *command,
    struct User_Context **pUserContext)
{

    /*
     * Hints:
     * - Determine where in memory each executable segment will be placed
     * - Determine size of argument block and where it memory it will
     *   be placed
     * - Copy each executable segment into memory
     * - Format argument block in memory
     * - In the created User_Context object, set code entry point
     *   address, argument block address, and initial kernel stack pointer
     *   address
     * 
     * - project1/src/geekos/lgrog.c
     */

	unsigned long virtSize = 0;
	unsigned int numArgs = 0, i = 0, ret = 0;
	ulong_t argBlockSize = 0;
	ulong_t maxva = 0;	/*maximun virtual addres*/
	ulong_t argBlockaddr = 0; 
	struct Exe_Segment *segment = NULL;
	
	/* Find maximum virtual address - scan Exec_Format*/
	maxva = Find_Maximum_Virtual_Address(exeFormat);
	/* 
	* -find args size
	* -setup some memory space for the program 
	*/
	Get_Argument_Block_Size(command, &numArgs,  &argBlockSize);
	virtSize = Round_Up_To_Page(maxva) + Round_Up_To_Page(DEFAULT_USER_STACK_SIZE + argBlockSize);
	argBlockaddr = Round_Up_To_Page(maxva) + DEFAULT_USER_STACK_SIZE; 

	(*pUserContext) = Create_User_Context(virtSize);
	
	if ((*pUserContext) != NULL){
		for (i = 0; i < exeFormat->numSegments; ++i){
			segment = &exeFormat->segmentList[i];
			if (segment != NULL){
				memcpy( (*pUserContext)->memory  + segment->startAddress,
					exeFileData + segment->offsetInFile,
					segment->lengthInFile);
		   }
		}
		
		/* - Format argument block in memory*/
		Format_Argument_Block((*pUserContext)->memory + argBlockaddr,
				numArgs, argBlockaddr , command);
		(*pUserContext)->entryAddr =  exeFormat->entryAddr;
		(*pUserContext)->argBlockAddr = argBlockaddr;
		(*pUserContext)->stackPointerAddr = argBlockaddr;

		
		DEBUG_PRINT("Load_User_Program: \n");
		DEBUG_PRINT(" virtSpace    = %x\n", (unsigned int) (*pUserContext)->memory);
		DEBUG_PRINT(" virtSize     = %x\n", (unsigned int) virtSize);
		DEBUG_PRINT(" codeSelector = %x\n", (*pUserContext)->csSelector);
		DEBUG_PRINT(" dataSelector = %x\n", (*pUserContext)->dsSelector);

	}else{
		DEBUG_PRINT("Load_User_Program: Error - Create_User_Context()\n");
		ret = -1;
	} 

	return ret;
}

/*
 * Copy data from user memory into a kernel buffer.
 * Params:
 * destInKernel - address of kernel buffer
 * srcInUser - address of user buffer
 * bufSize - number of bytes to copy
 *
 * Returns:
 *   true if successful, false if user buffer is invalid (i.e.,
 *   doesn't correspond to memory the process has a right to
 *   access)
 */
bool Copy_From_User(void* destInKernel, ulong_t srcInUser, ulong_t bufSize)
{
    /*
     * Hints: 
     * - the User_Context of the current process can be found
     *   from g_currentThread->userContext
     * - the user address is an index relative to the chunk
     *   of memory you allocated for it
     * - make sure the user buffer lies entirely in memory belonging
     *   to the process
     */
	bool mem_valid = false;

	if(g_currentThread->userContext != NULL){	
		mem_valid = Validate_User_Memory(g_currentThread->userContext,srcInUser,bufSize);
		if (mem_valid){
				memcpy(destInKernel,g_currentThread->userContext->memory+srcInUser,bufSize);
		}
		DEBUG_PRINT("Copy_From_User: bufSize = %i\n \
					useraddr = %p\n,kerneladdr = %p\n",
					(int)bufSize,
					g_currentThread->userContext->memory+srcInUser,
					destInKernel);
	}				
	return mem_valid; 
}

/*
 * Copy data from kernel memory into a user buffer.
 * Params:
 * destInUser - address of user buffer
 * srcInKernel - address of kernel buffer
 * bufSize - number of bytes to copy
 *
 * Returns:
 *   true if successful, false if user buffer is invalid (i.e.,
 *   doesn't correspond to memory the process has a right to
 *   access)
 */
bool Copy_To_User(ulong_t destInUser, void* srcInKernel, ulong_t bufSize)
{
    /* Hints: same as for Copy_From_User()*/
	bool mem_valid = false;
  
	if(g_currentThread->userContext != NULL){
		
		mem_valid = Validate_User_Memory(g_currentThread->userContext,destInUser,bufSize);
		if (mem_valid){
			memcpy(g_currentThread->userContext->memory+destInUser,srcInKernel,bufSize);
		}

		DEBUG_PRINT("Copy_From_User: bufSize = %i\n \
					useraddr = %p\n,kerneladdr = %p\n",
					(int)bufSize,
					g_currentThread->userContext->memory+destInUser,
					srcInKernel);
	}		
	return mem_valid;
}

/*
 * Switch to user address space belonging to given
 * User_Context object.
 * Params:
 * userContext - the User_Context
 */
void Switch_To_Address_Space(struct User_Context *userContext)
{
    /*
     * Hint: you will need to use the lldt assembly language instruction
     * to load the process's LDT by specifying its LDT selector.
     */
     
	__asm__ __volatile__ ("lldt %0"
							:
							: "a" (userContext->ldtSelector)
						 );
}

