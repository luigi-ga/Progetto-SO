#include "read_proc.h"

// lg
void get_uptime(double *uptime) {
    // open uptime file; check for errors in fopen 
    FILE *fp = fopen("/proc/uptime", "r");   
    if(!fp) handle_error("ERROR (get_uptime): fopen /proc/uptime file");
    // get values of loadavg and side effect on loads
    fscanf(fp, "%lf", uptime);
    // close loadavg file
    if(fclose(fp)) handle_error("ERROR (get_uptime): fclose /proc/uptime file");
}

/* Useful links:
 * https://man7.org/linux/man-pages/man2/sysinfo.2.html
 * https://stackoverflow.com/questions/39066998/what-are-the-meaning-of-values-at-proc-pid-stat */
void get_procinfo(ProcInfo *proc, int pid) {
    // lg
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

    // aa
    sprintf(filename,"/proc/%d/statm",pid);
    fp = fopen(filename,"r");
    if (!fp) handle_error("ERROR (get_procinfo): fopen /proc/[pid]/statm file");
    fscanf(fp, "%*d %ld %*d %*d %*d %ld %*d",
        &proc->statm_resident,
        &proc->statm_data);
    fclose(fp);
}