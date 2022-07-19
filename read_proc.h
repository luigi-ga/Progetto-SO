#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MiB(mem) (float) mem/(1<<10)

#define handle_error(msg)   do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct LoadAvg {
    float load0;
    float load1;
    float load2;
} LoadAvg;

/* https://stackoverflow.com/questions/39066998/what-are-the-meaning-of-values-at-proc-pid-stat */
typedef struct ProcInfo {
    // lg
    unsigned pid;
    unsigned uid;
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
    // aa
    long int resident;
    long int shared;
    long int data; 
} ProcInfo;

// aa
typedef struct MemInfo {
    unsigned long memTotal;
    unsigned long memFree;
    unsigned long buffers;
    unsigned long cached;
    unsigned long swapTotal;
    unsigned long swapFree;
    unsigned long memAvailable;
} MemInfo;

// aa
typedef struct CpuInfo {
    double user;
    double nice;
    double system;
    double idle;
    double iowait;
    double irq;
    double softirq;
    double steal;
    double guest;
} CpuInfo;

// lg
int get_uptime(double *uptime);
int get_loadavg(LoadAvg *loads);

// aa
int get_cpuinfo(CpuInfo *cpu);

// aa
int get_meminfo(MemInfo *mem);

// lg, aa
int get_procinfo(ProcInfo *proc, int pid);

#endif