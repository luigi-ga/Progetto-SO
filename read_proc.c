#include "read_proc.h"

// lg
int get_uptime(double *uptime) {
    // check if uptime have been initialized
    if (uptime == NULL) handle_error("ERROR (get_uptime): uptime not intialized");
    // open uptime file
    FILE *fp = fopen("/proc/uptime", "r");   
    if (!fp) return -1;
    // get values of loadavg and side effect on loads
    fscanf(fp, "%lf", uptime);
    // close uptime file
    if (fclose(fp)) handle_error("ERROR (get_uptime): fclose /proc/uptime file");
    return 0;
}

// lg
int get_loadavg(LoadAvg *loads) {    
    // check if loads have been initialized
    if (loads == NULL) handle_error("ERROR (get_loadavg): loads not initialized");
    // open loadavg file
    FILE *fp = fopen("/proc/loadavg", "r");
    if (!fp) return -1;
    // get values of loadavg and side effect on loads
    fscanf(fp, "%f %f %f", &loads->load0, &loads->load1, &loads->load2);
    // close loadavg file
    if (fclose(fp)) handle_error("ERROR (get_loadavg): fclose /proc/loadavg file");
    return 0;
}

// aa 
int get_cpuinfo(CpuInfo *cpu) {
    // check if cpu have been initialized
    if (cpu == NULL) handle_error("ERROR (get_cpuinfo): cpu not initialized");
    // open stat file
    FILE *fp = fopen("/proc/stat","r");
    if (!fp) return -1;
    // get stat values
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
    // close stat file
    if (fclose(fp)) handle_error("ERROR (get_cpuinfo): fclose /proc/stat file");
    return 0;
}

// aa
int get_meminfo(MemInfo *mem) {
    ssize_t read, len = 0;
    char *line;
    // check if mem has been initialized
    if (mem == NULL) handle_error("ERROR (get_meminfo): mem not initialized");
    // open meminfo file
    FILE *fp = fopen("/proc/meminfo","r");
    if (!fp) return -1; 
    // get values and side effect on mem
    while ((read = getline(&line, (long unsigned int*) &len, fp)) != -1) {
        if (sscanf(line, "MemTotal: %ld kB", &mem->memTotal)) continue;
        if (sscanf(line, "MemFree: %ld kB", &mem->memFree)) continue;
        if (sscanf(line, "MemAvailable: %ld kB",&mem->memAvailable)) continue;
        if (sscanf(line, "Buffers: %ld kB",&mem->buffers)) continue;
        if (sscanf(line, "Cached: %ld kB",&mem->cached)) continue;
        if (sscanf(line, "SwapTotal: %ld kB",&mem->swapTotal)) continue;
        // if sscanf matches with SwapFree (the last term we need), we break the loop
        if (sscanf(line, "SwapFree: %ld kB",&mem->swapFree)) break;
    }
    // close meminfo file
    if (fclose(fp)) handle_error("ERROR (get_meminfo): fclose /proc/meminfo file");
    return 0;
}

/* https://man7.org/linux/man-pages/man2/sysinfo.2.html
 * https://stackoverflow.com/questions/39066998/what-are-the-meaning-of-values-at-proc-pid-stat 
 * https://man7.org/linux/man-pages/man5/proc.5.html*/
int get_procinfo(ProcInfo *proc, int pid) {
    // lg | check if proc have been initialized
    if (proc == NULL) handle_error("ERROR (get_procinfo): proc not initialized");
    // write in filename the directory
    char filename[300];
    sprintf(filename, "/proc/%d/stat", pid);   
    // open stat file 
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1; 
    // get stat info
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
    // close stat file            
    if (fclose(fp)) handle_error("ERROR (get_procinfo): fclose /proc/[pid]/stat file");

    ssize_t read, len = 0;
    char *line;
    // write in filename the directory
    sprintf(filename, "/proc/%d/status", pid);   
    // open status file 
    fp = fopen(filename, "r");
    if (!fp) return -1; 
    // get status info
    while ((read = getline(&line, (long unsigned int*) &len, fp)) != -1) {
        if (sscanf(line, "Uid: %d %*d %*d %*d\n", &proc->uid)) break;
    }
    // close status file
    if (fclose(fp)) handle_error("ERROR (get_procinfo): fclose /proc/[pid]/status file");

    // aa
    // write in filename the directory
    sprintf(filename,"/proc/%d/statm",pid);
    // open statm file
    fp = fopen(filename,"r");
    if (!fp) return -1;
    // get statm info
    fscanf(fp, "%*d %ld %ld %*d %*d %ld %*d",
        &proc->resident,
        &proc->shared,
        &proc->data);
    // close statm file    
    if (fclose(fp)) handle_error("ERROR (get_procinfo): fclose /proc/[pid]/stat file");
    return 0;
}