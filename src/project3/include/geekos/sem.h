#ifndef GEEKOS_SEM_H
#define GEEKOS_SEM_H

#define MAX_SEM 20
#define MAX_NAME_SEM 25

#include <geekos/list.h>
#include <geekos/kthread.h>


struct Semaphore;

int Create_Semaphore(char *name, int ival);
int P(int sem);
int V(int sem);
int Destroy_Semaphore(int sem);

#endif

