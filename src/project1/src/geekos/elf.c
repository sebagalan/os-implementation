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
int Parse_ELF_Executable(char *exeFileData, ulong_t exeFileLength,
    struct Exe_Format *exeFormat)
{

	int i=0, offset=0;
	
	elfHeader *elfh = NULL;
	programHeader *ph = NULL;
	struct Exe_Segment *s = NULL;

	exeFormat = Malloc(sizeof(struct Exe_Format));
	KASSERT(exeFormat != NULL);
	memset(exeFormat,'\0',sizeof(struct Exe_Format));
	KASSERT(exeFormat != NULL);
	
	elfh = Malloc(sizeof(elfHeader));
	KASSERT(elfh != NULL);
	memset(elfh,0,sizeof(elfHeader));
	KASSERT(elfh != NULL);

	elfh = memcpy(elfh,exeFileData,sizeof(elfHeader));
	
/*	Print("file size %u\n",exeFileLength);
	Print("%s\n",elfh	->	ident);
	Print("elfh ->	type %i\n",elfh	->	type);
	Print("elfh ->	machine %i\n",elfh	->	machine);
	Print("elfh ->	version %i\n",elfh	->	version);
	Print("elfh ->	entry %i\n",elfh	->	entry);
	Print("elfh ->	phoff %i\n",elfh	->	phoff);
	Print("elfh ->	sphoff %i\n",elfh	->	sphoff);
	Print("elfh ->	flags %i\n",elfh	->	flags);
	Print("elfh ->	ehsize %i\n",elfh	->	ehsize);
	Print("elfh ->	phentsize %i\n",elfh	->	phentsize);
	Print("elfh ->	phnum %i\n",elfh	->	phnum);
	Print("elfh ->	shentsize %i\n",elfh	->	shentsize);
	Print("elfh ->	shnum %i\n",elfh	->	shnum);
	Print("elfh ->	shstrndx %i\n",elfh	->	shstrndx);*/

	offset = elfh->phoff;

	for(i=0;i< elfh->phnum; i++){

		ph =  (programHeader *)(exeFileData+offset);
		Print("ph->type %u ",ph -> type);
		Print("ph->offset %u ",ph -> offset);
		Print("ph->vaddr %u ",ph -> vaddr);
		Print("ph->paddr %u",ph -> paddr);
		Print("ph->fileSize %u",ph -> fileSize);
		Print("ph->memSize %u",ph -> memSize);
		Print("ph->flags %u",ph -> flags);
		Print("ph->alignment %u\n",ph -> alignment);
	
		s = Malloc(sizeof(struct Exe_Segment));
		memset(s,0,sizeof(struct Exe_Segment));
		
		s->offsetInFile= ph->offset;	 
		s->lengthInFile = ph->fileSize;	 
		s->startAddress = ph->vaddr;
		s->sizeInMemory = ph->memSize;
		s->protFlags = ph -> flags;
		exeFormat->segmentList[i] = *s;
		offset = offset+32;
	}
	
	exeFormat->numSegments = elfh->phnum ;
	exeFormat->entryAddr = elfh->entry;	 	
	/*No elimino s, porque quiero usarlo mas tarde...*/
    /*TODO("Parse an ELF executable image");*/

	return 0;
}

