#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

#define MiB(mem) (float) mem/(1<<10)

#define handle_error(msg)   do { perror(msg); exit(EXIT_FAILURE); } while (0)


/* https://stackoverflow.com/questions/39066998/what-are-the-meaning-of-values-at-proc-pid-stat */
typedef struct ProcInfo {
    unsigned pid;
    char command[20];
    char state;
    unsigned long utime;
    unsigned long stime;
    unsigned long cutime;
    unsigned long cstime;
    long long starttime;
    long int priority;
    long int nice;
    unsigned long virt;
    long int res;
} ProcInfo;


void get_procinfo(ProcInfo *proc, int pid);


#endif