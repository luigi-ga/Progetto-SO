#include "read_proc.h"

/* function similar to https://man7.org/linux/man-pages/man2/sysinfo.2.html */
void get_procinfo(ProcInfo *proc, int pid) {
    char filename[300];
    sprintf(filename, "/proc/%d/stat", pid);    
    FILE *fp = fopen(filename, "r");
    if(!fp) handle_error("ERROR (get_procinfo): fopen /proc/[pid]/stat file");
    
    fscanf(fp, "%d %s %c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu %lu %lu %lu %ld %ld %*d %*d %lld %lu %ld", 
            &proc->pid, 
            proc->command, 
            &proc->state,
            &proc->utime,
            &proc->stime,
            &proc->cutime,
            &proc->cstime,
            &proc->priority,
            &proc->nice,            
            &proc->starttime,
            &proc->virt,
            &proc->res);
            
    fclose(fp);
}