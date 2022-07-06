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
    long int statm_resident;
    long int statm_data; 
} ProcInfo;

// aa
typedef struct MemInfo {
    unsigned long memTotal;
    unsigned long memFree;
    unsigned long buffers;
    unsigned long swapTotal;
    unsigned long swapFree;
    unsigned long memAvailable;
} MemInfo;

//aa
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

//lg
void get_uptime(double *uptime);
void get_loadavg(LoadAvg *loads);

//aa
void get_meminfo(MemInfo *mem);

//aa
void get_cpuinfo(CpuInfo *cpu);

// lg, aa
void get_procinfo(ProcInfo *proc, int pid);

#endif