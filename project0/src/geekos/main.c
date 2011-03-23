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
        int Row = 0;
        int Col = 0;

		Print("Welcome\n");
		/*seguir leyendo screen.c/.h*/
	        while(1){
			Code = Wait_For_Key();
			if( !(Code & KEY_RELEASE_FLAG) ){
				switch ((int)Code){
					case 13: /*en ascii 13 es Enter*/
						Get_Cursor(&Row,&Col);
						Put_Cursor(Row+1,0);
					break;
					case ASCII_BS: 
						/*vulelve el cursor, pero no puedo hacer que borre
						 * el caracter %&$°!)$#"= */
						Get_Cursor(&Row,&Col);
						Put_Cursor(Row,Col-1);
					break;
					default:
						Print("%c",Code);
					break;
				}
			}
		}
}

 
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


/*	Set_Current_Attr(ATTRIB(BLACK, GREEN|BRIGHT));
 Print("Welcome to GeekOS!\n");*/
    Start_Kernel_Thread((Thread_Start_Func)&Main_Thread,0,PRIORITY_NORMAL,false);
/*  Set_Current_Attr(ATTRIB(BLACK, GRAY));
    TODO("Start a kernel thread to echo pressed keys and print counts");*/
    /* Now this thread is done. */
    Exit(0);
}









