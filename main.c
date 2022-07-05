#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <fnmatch.h>

#include "read_proc.h"

int print = 1;

// lg
void sig_handler(int signum) {
    print = 1;
}

// aa
int filter(const struct dirent *dir) {
    return !fnmatch("[1-9]*",dir->d_name,0);
}


/* Useful links:
 * https://linuxaria.com/howto/understanding-the-top-command-on-linux?lang=it ,  
   https://www.baeldung.com/linux/total-process-cpu-usage */
void print_top() {
    write(STDOUT_FILENO, "\e[1;1H\e[2J", 11); 

    // lg
    time_t rawtime;
    time(&rawtime);    
    struct tm *time = localtime(&rawtime);
    LoadAvg *loads = (LoadAvg*)malloc(sizeof(LoadAvg));
    get_loadavg(loads);
    double uptime;
    get_uptime(&uptime);
    printf("top - %.2d:%.2d:%.2d up %d min, 0 users, load average: %.2f, %.2f, %.2f\n", 
            time->tm_hour, time->tm_min, time->tm_sec,
            (int) uptime/60, 
            loads->load0, loads->load1, loads->load2);

    // lg
    MemInfo *mem = (MemInfo*)malloc(sizeof(MemInfo));
    get_meminfo(mem);
    printf("MiB Mem: %.1f total, %.1f free, %.1f used, %.1f buff/cache\n", 
            MiB(mem->memTotal), 
            MiB(mem->memFree), 
            MiB((mem->memTotal - mem->memFree)), 
            MiB(mem->buffers));
    printf("MiB Swap: %.1f total, %.1f free, %.1f used, %.1f avail Mem\n", 
            MiB(mem->swapTotal), 
            MiB(mem->swapFree), 
            MiB((mem->swapTotal - mem->swapFree)), 
            MiB(mem->memAvailable));

    // aa
    // CICLO SUI PID
    printf("\n%c[%d;%dmPID       |USER  |PR   |NI   |VIRT      |RES  |SH |S    |%%CPU |%%MEM |TIME+   |COMMAND%c[%dm\n",27,1,35,27,0);
    struct dirent **namelist;
    ProcInfo *proc = (ProcInfo*)malloc(sizeof(ProcInfo));
    // prendiamo dalla directory proc solo le directory il cui nome sia un numero
    // in modo da prendere solo le cartelle dei processi
    int n = scandir("/proc", &namelist, filter, alphasort);
    if (n == -1) handle_error("ERROR (print_top): scandir");
    while (n--) {
        // richiama la funzione get_procinfo su ogni processo
        get_procinfo(proc,atoi(namelist[n]->d_name));
        printf("%-10d| user |%-5ld|%-5ld|%-10lu|%-5ld|SHR|%-5c|%-5.1f|%-5.1f|0:%-6.2f|%s\n",
                proc->pid,
                proc->priority,
                proc->nice,
                (long int) MiB(proc->virt),
                proc->res,
                proc->state,
                (float)  (((proc->utime/100) + (proc->stime/100))*100 / (uptime - (proc->starttime / 100))),
                (float) ((proc->statm_resident + proc->statm_data)*100) / (mem->memTotal),
                (float) (proc->utime + proc->stime) / 100,
                proc->command);     
    }





    free(proc);    
    free(loads);
    free(mem);   
    alarm(1);
}


int main() {
    signal(SIGALRM, sig_handler); // Register signal handler
    
    while(1) {
        if(print) {
            print = 0;
            print_top();
        } 
    }
    return 0;
}