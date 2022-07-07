#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <fnmatch.h>

#include "read_proc.h"

#define RUNNING 82
#define SLEEPING 83
#define STOPPED 84
#define ZOMBIE 90

#define CYAN "\e[1m\033[0;36m"
#define WHITE "\033[0;37m"

int print = 1;
unsigned int total_p = 0;
unsigned int running_p = 0;
unsigned int sleeping_p = 0;
unsigned int stopped_p = 0;
unsigned int zombie_p = 0;

// lg
void sig_handler(int signum) {
    print = 1;
}

// aa
int filter(const struct dirent *dir) {
    return !fnmatch("[1-9]*",dir->d_name,0);
}

/* https://linuxaria.com/howto/understanding-the-top-command-on-linux?lang=it 
   https://www.idnt.net/en-US/kb/941772 
   https://www.baeldung.com/linux/total-process-cpu-usage 
   https://stackoverflow.com/questions/2347770/how-do-you-clear-the-console-screen-in-c */
void print_top() {
    // clean stdout
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
    printf("Tasks: %d total, %d running, %d sleeping, %d stopped, %d zombie\n",
                      total_p,  running_p,  sleeping_p,  stopped_p,  zombie_p);
    total_p = running_p = sleeping_p = stopped_p = zombie_p = 0;

    // aa
    CpuInfo *cpu = (CpuInfo*)malloc(sizeof(CpuInfo));
    get_cpuinfo(cpu);
    long int cpu_sum = (cpu->user)+(cpu->system)+(cpu->nice)+(cpu->idle)+(cpu->iowait)+(cpu->irq)+(cpu->softirq)+(cpu->steal)+(cpu->guest);
    printf("%%Cpu(s): %.1f us, %.1f sy, %.1f ni, %.1f id, %.1f wa, %.1f hi, %.1f si, %.1f st\n",
            (100* (cpu->user/cpu_sum)),
            (100* (cpu->system/cpu_sum)),
            (100* (cpu->nice/cpu_sum)),
            (100* (cpu->idle/cpu_sum)),
            (100* (cpu->iowait/cpu_sum)),
            0.0,
            (100* (cpu->softirq/cpu_sum)),
            (100* (cpu->steal/cpu_sum)));

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
    // https://stackoverflow.com/questions/58314879/terminal-background-color-not-always-properly-reset-using-0330m
    printf("\033[46m\n%6s %-8s %3s %2s %8s %8s %8s %2s %6s %6s %8s %-5s %c\n\e[0m",
            "PID", "USER", "PR", "NI", "VIRT", "RES", "SHR", "S", "\%CPU", "\%MEM", "TIME+", "COMMAND", 27);
    struct dirent **namelist;
    ProcInfo *proc = (ProcInfo*)malloc(sizeof(ProcInfo));
    // prendiamo dalla directory proc solo le directory il cui nome sia un numero
    // in modo da prendere solo le cartelle dei processi
    int n = scandir("/proc", &namelist, filter, alphasort);
    if (n == -1) handle_error("ERROR (print_top): scandir");
    while (n--) {
        // richiama la funzione get_procinfo su ogni processo
        get_procinfo(proc, atoi(namelist[n]->d_name));
        char *color = WHITE;
        if(proc->state == RUNNING) color = CYAN;
        printf("%s%6d %-8s %3ld %2ld %8lu %8ld %8ld %2c %6.1f %6.1f %8.2f %-5s\e[0m\n",
                color,
                proc->pid,
                "user",
                proc->priority,
                proc->nice,
                proc->virt / (1<<10),
                proc->res,
                proc->shared * (1<<2),
                proc->state,
                (float)  ((proc->utime + proc->stime) / (uptime - (proc->starttime / 100))),
                (float) ((proc->resident + proc->data) * 100) / (mem->memTotal),
                (float) (proc->utime + proc->stime) / 100,
                proc->command);   
            
        // lg
        total_p += 1;
        switch(proc->state) {
            case(RUNNING):  running_p += 1; break;
            case(SLEEPING): sleeping_p += 1; break;
            case(STOPPED):  stopped_p += 1; break;
            case(ZOMBIE):   zombie_p += 1; break;
        }          
    }
       
    free(loads);
    free(cpu);
    free(mem);  
    free(proc);  
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