/*
 * A test program for GeekOS user mode
 */

#include <conio.h>
#include <geekos/syscall.h>
#include <string.h>
 
void foo (char *bar)
{
   char  c[12];
 
   strcpy(c, bar); 
}
 
int main (int argc, char **argv){
	
	foo(argv[1]); 
	return 0;

}



