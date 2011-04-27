/*
 * A test program for GeekOS user mode
 */

#include <conio.h>
#include <geekos/syscall.h>
#include <string.h>
 
#define TAM 1<<12 

int foo(void) {
	/*cuanto aguanta el stack*/
	foo();
	return 0;
}
 
int foo2(void) {

	int i=0;	
	int *ptr = NULL;

	for(i=0;i<TAM;i++){
		Print("%x\n", *ptr);
		ptr = (int*) (((int)&ptr) * TAM);
		*ptr = (int)&ptr;
	}	 
	return (int)ptr;
} 
 
int main (int argc, char **argv){
	foo2();
	return 0;
}



