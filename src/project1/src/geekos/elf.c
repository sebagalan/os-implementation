/*
 * ELF executable loading
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2003, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.29 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/errno.h>
#include <geekos/kassert.h>
#include <geekos/ktypes.h>
#include <geekos/screen.h>  /* for debug Print() statements */
#include <geekos/pfat.h>
#include <geekos/malloc.h>
#include <geekos/string.h>
#include <geekos/elf.h>


/**
 * From the data of an ELF executable, determine how its segments
 * need to be loaded into memory.
 * @param exeFileData buffer containing the executable file
 * @param exeFileLength length of the executable file in bytes
 * @param exeFormat structure describing the executable's segments
 *   and entry address; to be filled in
 * @return 0 if successful, < 0 on error
 */


/*typedef enum e_tag {
	PT_NULL=0, PT_LOAD , PT_DYNAMIC ,
	PT_INTERP,PT_NOTE,PT_SHLIB,
	PT_PHDR, PT_LOPROC=0x70000000,
	PT_IOPROC = 0x7fffffff} P_TYPE;*/

int Parse_ELF_Executable(char *exeFileData, ulong_t exeFileLength,
    struct Exe_Format *exeFormat){

	int i=0, offset=0;
	elfHeader *elfh = NULL;
	programHeader *ph = NULL;

	KASSERT(exeFormat!=NULL);
	memset(exeFormat,0,sizeof(struct Exe_Format));
		
	elfh = (elfHeader*) exeFileData;
	offset = elfh->phoff;

	for(i=0;i< elfh->phnum; i++){
		ph =  (programHeader *)(exeFileData+offset);
		exeFormat->segmentList[i].offsetInFile= ph->offset;	 
		exeFormat->segmentList[i].lengthInFile = ph->fileSize;	 
		exeFormat->segmentList[i].startAddress = ph->vaddr;
		exeFormat->segmentList[i].sizeInMemory = ph->memSize;
		exeFormat->segmentList[i].protFlags = ph->flags;
		offset = offset+32;
	}

	exeFormat->numSegments = elfh->phnum ;
	exeFormat->entryAddr = elfh->entry;	 	
    /*TODO("Parse an ELF executable image");*/
	
	return 0;
}

