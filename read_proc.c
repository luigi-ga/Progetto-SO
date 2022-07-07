#include "read_proc.h"

// lg
void get_uptime(double *uptime) {
    // open uptime file; check for errors in fopen 
    FILE *fp = fopen("/proc/uptime", "r");   
    if (!fp) handle_error("ERROR (get_uptime): fopen /proc/uptime file");
    // get values of loadavg and side effect on loads
    fscanf(fp, "%lf", uptime);
    // close loadavg file
    if (fclose(fp)) handle_error("ERROR (get_uptime): fclose /proc/uptime file");
}

// lg
void get_loadavg(LoadAvg *loads) {    
    // check if loads have been initialized
    if (loads == NULL) handle_error("ERROR (get_loadavg): loads not initialized");
    // open loadavg file; check for errors in fopen
    FILE *fp = fopen("/proc/loadavg", "r");
    if (!fp) handle_error("ERROR (get_loadavg): fopen /proc/loadavg file");
    // get values of loadavg and side effect on loads
    fscanf(fp, "%f %f %f", &loads->load0, &loads->load1, &loads->load2);
    // close loadavg file
    if (fclose(fp)) handle_error("ERROR (get_loadavg): fclose /proc/loadavg file");
}

// aa 
void get_cpuinfo(CpuInfo *cpu) {
    FILE *fp = fopen("/proc/stat","r");
    if (!fp) handle_error("ERROR (get_cpuinfo): fopen /proc/stat file");
    fscanf(fp,"%*s %lf %lf %lf %lf %lf %lf %lf %lf %lf %*d",
        &cpu->user,
        &cpu->nice,
        &cpu->system,
        &cpu->idle,
        &cpu->iowait,
        &cpu->irq,
        &cpu->softirq,
        &cpu->steal,
        &cpu->guest
    );
    fclose(fp);
}

// aa
void get_meminfo(MemInfo *mem) {
    ssize_t read, len = 0;
    char *line;
    // check if mem has been initialized
    if (mem == NULL) handle_error("ERROR (get_meminfo): mem not initialized");
    // open meminfo file and check errors in fopen
    FILE *fp = fopen("/proc/meminfo","r");
    if (!fp) handle_error("ERROR (get_meminfo): fopen /proc/meminfo file");
    // get values and side effect on mem
    while ((read = getline(&line, (long unsigned int*) &len, fp)) != -1) {
        if (sscanf(line, "MemTotal: %ld kB", &mem->memTotal)) continue;
        if (sscanf(line, "MemFree: %ld kB", &mem->memFree)) continue;
        if (sscanf(line, "MemAvailable: %ld kB",&mem->memAvailable)) continue;
        if (sscanf(line, "Buffers: %ld kB",&mem->buffers)) continue;
        if (sscanf(line, "SwapTotal: %ld kB",&mem->swapTotal)) continue;
        // if sscanf matches with SwapFree (the last term we need), we break the loop
        if (sscanf(line, "SwapFree: %ld kB",&mem->swapFree)) break;
    }
    // close meminfo file
    if (fclose(fp)) handle_error("ERROR (get_meminfo): fclose /proc/meminfo file");
}

/* https://man7.org/linux/man-pages/man2/sysinfo.2.html
 * https://stackoverflow.com/questions/39066998/what-are-the-meaning-of-values-at-proc-pid-stat 
 * https://man7.org/linux/man-pages/man5/proc.5.html*/
void get_procinfo(ProcInfo *proc, int pid) {
    // lg
    char filename[300];
    sprintf(filename, "/proc/%d/stat", pid);    
    FILE *fp = fopen(filename, "r");
    if (!fp) handle_error("ERROR (get_procinfo): fopen /proc/[pid]/stat file");    
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
    fscanf(fp, "%*d %ld %ld %*d %*d %ld %*d",
        &proc->resident,
        &proc->shared,
        &proc->data);
    fclose(fp);
}