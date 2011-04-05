/*
 *  A simple program for testing ELF parsing and loading
 *  
 * Copyright (c) 2004, David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2004, Iulian Neamtiu <neamtiu@cs.umd.edu>
 * $Revision: 1.24 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

/* ELF_Print only prints NULL-terminated strings; 
 * no formatting or other fancy features */
void ELF_Print(char* msg);

/*s2 se imprime si cambio su definicion. En lugar que sea 
 * char s2[40], tiene que ser char *s2 para que funcione.
 * Si es char *s2 entonces s2 figura dentro de la seccion
 * .rodata (solo lectura). Si s2 es char s2[40], figura
 * dentro de .text (aunque deberia ser parte del stack).
 
 * Es una variable inicializada ... ¿Porque esta dentro de text?
 * ¿Tendra que ver la tabla de simbolos, o las secciones?*/


char  s1[40] = "Hi ! This is the first string\n";

int main(int argc, char** argv)
{
/*	char  s2[40] = "Hi ! This is the second string\n"; */
	char  *s2 = "Hi ! This is the second string\n";

	ELF_Print(s1);
	ELF_Print(s2); 
   return 0;
}
