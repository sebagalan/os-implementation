/*
 * GeekOS C code entry point
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2004, Iulian Neamtiu <neamtiu@cs.umd.edu>
 * $Revision: 1.51 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/bootinfo.h>
#include <geekos/string.h>
#include <geekos/screen.h>
#include <geekos/mem.h>
#include <geekos/crc32.h>
#include <geekos/tss.h>
#include <geekos/int.h>
#include <geekos/kthread.h>
#include <geekos/trap.h>
#include <geekos/timer.h>
#include <geekos/keyboard.h>




/*
 * Kernel C code entry point.
 * Initializes kernel subsystems, mounts filesystems,
 * and spawns init process.
 */

void Main_Thread(void){

	Keycode Code = KEY_UNKNOWN;
	Print("Hola vo!\n");
	
	/*KEY_CTRL_FLAG es una mascara, si le aplico XOR tendo el Keycode
	 * de lo de la tecla que aprete..*/
	Code = Wait_For_Key(); 
	while( !((Code^KEY_CTRL_FLAG)==(uchar_t)'d') ){
		if ((Code&KEY_RELEASE_FLAG) == 0){
			Print("%c",Code);
		}
		Code = Wait_For_Key();
	}
	Print("Ctrl+d: no escribis mas ... \n");
}

/*
El assert salta si quito el flag -O del Makefile ( los incrementos no
* son atomicos, pero con -O no hace incrementos -no vi add o sub-)
unsigned int x=0;
void Thread_h1(){ while(1){x++; x--;}  }
void Thread_h2(){ while(1){x++; x--;}  }
void Thread_Kassert(){ while(1){KASSERT( (0 <= x) && (x <= 2));} }*/

/*Con Yield() imprime tanto a,b mezclado. Sin Yield() imprime muchas
 * veces a y despues muchas veces b y asi..
void Thread_h1(){ while(1){Print("a\n");Yield();} }
void Thread_h2(){ while(1){Print("b\n");Yield();} }*/


void Main(struct Boot_Info* bootInfo)
{
    Init_BSS();
    Init_Screen();
    Init_Mem(bootInfo);
    Init_CRC32();
    Init_TSS();
    Init_Interrupts();
    Init_Scheduler();
    Init_Traps();
    Init_Timer();
    Init_Keyboard();

/*  Set_Current_Attr(ATTRIB(BLACK, GREEN|BRIGHT));
    Print("Welcome to GeekOS!\n");
    Set_Current_Attr(ATTRIB(BLACK, GRAY));
	TODO("Start a kernel thread to echo pressed keys and print counts");*/
	Start_Kernel_Thread((Thread_Start_Func)&Main_Thread,0,PRIORITY_NORMAL,false);
/*	Start_Kernel_Thread((Thread_Start_Func)&Thread_h1,0,PRIORITY_NORMAL,false);
	Start_Kernel_Thread((Thread_Start_Func)&Thread_h2,0,PRIORITY_NORMAL,false);
	Start_Kernel_Thread((Thread_Start_Func)&Thread_Kassert,0,PRIORITY_NORMAL,false);*/
    /* Now this thread is done. */
    Exit(0);
}









