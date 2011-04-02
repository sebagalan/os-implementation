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

	elfHeader *elfh = NULL;
	programHeader *ph = NULL;
	
	exeFormat = Malloc(sizeof(struct Exe_Format));
	KASSERT(exeFormat != NULL);
	memset(exeFormat,0,sizeof(struct Exe_Format));
	KASSERT(exeFormat != NULL);
	
	elfh = Malloc(sizeof(elfHeader));
	KASSERT(elfh != NULL);
	memset(elfh,0,sizeof(elfHeader));
	KASSERT(elfh != NULL);

	ph = Malloc(sizeof(programHeader));
	KASSERT(ph != NULL);
	memset(ph,0,sizeof(programHeader));
	KASSERT(ph != NULL);

	memcpy(elfh,exeFileData,sizeof(elfHeader));
	
	Print("file size %ul\n",exeFileLength);
	
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
	Print("elfh ->	shstrndx %i\n",elfh	->	shstrndx);

	memcpy(ph,exeFileData+elfh->phoff,(elfh->phnum*elfh->phentsize));

	Print("ph ->	type %u\n",ph -> type);
    Print("ph ->	offset %u\n",ph -> offset);
    Print("ph ->	vaddr %u\n",ph -> vaddr);
    Print("ph ->	paddr %u\n",ph -> paddr);
    Print("ph ->	fileSize %u\n",ph -> fileSize);
    Print("ph ->	memSize %u\n",ph -> memSize);
    Print("ph ->	flags %u\n",ph -> flags);
    Print("ph ->	alignment %u\n",ph -> alignment);

	Free(elfh);
	Free(ph);

    /*TODO("Parse an ELF executable image");*/

	return -1;
}

