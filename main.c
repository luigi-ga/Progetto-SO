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

#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define PURPLE  "\033[0;35m"
#define CYAN    "\033[0;36m"
#define WHITE   "\033[0;37m"

#define ALARMTIME 2

int print = 1, ask = 0;
unsigned int total_p = 0;
unsigned int running_p = 0;
unsigned int sleeping_p = 0;
unsigned int stopped_p = 0;
unsigned int zombie_p = 0;
struct dirent **namelist;
int len_namelist;

// lg
void sigalarm_handler(int signum) {
    print = 1;
    ask = 0;
}


//aa
void sigquit_handler(int sig_num) {
    /* Reset handler to catch SIGINT next time.
    Refer http://en.cppreference.com/w/c/program/signal */
    signal(SIGQUIT, sigquit_handler);
    ask = 1;
    print = 0;
}

// aa
int filter(const struct dirent *dir) {
    return !fnmatch("[1-9]*",dir->d_name,0);
}

/* https://linuxaria.com/howto/understanding-the-top-command-on-linux?lang=it  
   https://man7.org/linux/man-pages/man5/proc.5.html
   https://www.idnt.net/en-US/kb/941772 
   https://www.baeldung.com/linux/total-process-cpu-usage 
   https://stackoverflow.com/questions/2347770/how-do-you-clear-the-console-screen-in-c*/
void print_top() {
    // clean stdout
    write(STDOUT_FILENO, "\e[1;1H\e[2J", 11); 

    // lg
    time_t rawtime;
    time(&rawtime);    
    struct tm *time = localtime(&rawtime);
    LoadAvg *loads = (LoadAvg*)malloc(sizeof(LoadAvg));
    if (get_loadavg(loads)) handle_error("ERROR (print_top): get_loadavg");
    double uptime;
    if (get_uptime(&uptime)) handle_error("ERROR (print_top): get_uptime");
    printf("top - %.2d:%.2d:%.2d up %.2d:%.2d:%.2d, ??? users, load average: %.2f, %.2f, %.2f\n", 
            time->tm_hour, time->tm_min, time->tm_sec,
            (int) uptime/3600, (int) (uptime/60)%60, (int) uptime%60,
            loads->load0, loads->load1, loads->load2);
    
    // lg
    printf("Tasks: %d total, %d running, %d sleeping, %d stopped, %d zombie\n",
                      total_p,  running_p,  sleeping_p,  stopped_p,  zombie_p);
    total_p = running_p = sleeping_p = stopped_p = zombie_p = 0;

    // aa
    CpuInfo *cpu = (CpuInfo*)malloc(sizeof(CpuInfo));
    if (get_cpuinfo(cpu)) handle_error("ERROR (print_top): get_cpuinfo");
    long int cpu_sum = (cpu->user)+(cpu->system)+(cpu->nice)+(cpu->idle)+(cpu->iowait)+(cpu->irq)+(cpu->softirq)+(cpu->steal)+(cpu->guest);
    printf("%%Cpu(s): %.1f us, %.1f sy, %.1f ni, %.1f id, %.1f wa, %.1f hi, %.1f si, %.1f st\n",
            (100* (cpu->user/cpu_sum)),
            (100* (cpu->system/cpu_sum)),
            (100* (cpu->nice/cpu_sum)),
            (100* (cpu->idle/cpu_sum)),
            (100* (cpu->iowait/cpu_sum)),
            (100* (cpu->irq/cpu_sum)),
            (100* (cpu->softirq/cpu_sum)),
            (100* (cpu->steal/cpu_sum)));

    // lg
    MemInfo *mem = (MemInfo*)malloc(sizeof(MemInfo));
    if (get_meminfo(mem)) handle_error("ERROR (print_top): get_meminfo");
    printf("MiB Mem: %.1f total, %.1f free, %.1f used, %.1f buff/cache\n", 
            MiB(mem->memTotal), 
            MiB(mem->memFree), 
            MiB((mem->memTotal - mem->memFree - (mem->buffers + mem->cached))), 
            MiB((mem->buffers + mem->cached)));
    printf("MiB Swap: %.1f total, %.1f free, %.1f used, %.1f avail Mem\n", 
            MiB(mem->swapTotal), 
            MiB(mem->swapFree), 
            MiB((mem->swapTotal - mem->swapFree)), 
            MiB(mem->memAvailable));

    // aa
    // CICLO SUI PID
    // https://stackoverflow.com/questions/58314879/terminal-background-color-not-always-properly-reset-using-0330m
    printf("\033[46m\n%6s %-6s %3s %2s %8s %8s %8s %2s %6s %6s %9s %s %c\e[0m\n",
            "PID", "UID", "PR", "NI", "VIRT", "RES", "SHR", "S", "\%CPU", "\%MEM", "TIME+", "COMMAND", 27);
    
    ProcInfo *proc = (ProcInfo*)malloc(sizeof(ProcInfo));
    // prendiamo dalla directory proc solo le directory il cui nome sia un numero
    // in modo da prendere solo le cartelle dei processi
    int timesec, n = scandir("/proc", &namelist, filter, alphasort);
    len_namelist = n;
    if (n == -1) handle_error("ERROR (print_top): scandir");
    while (n--) {
        // richiama la funzione get_procinfo su ogni processo
        if (get_procinfo(proc, atoi(namelist[n]->d_name))) continue;
        char *color = WHITE;
        if (proc->state == RUNNING) color = CYAN;
        else if (proc->state == ZOMBIE) color = PURPLE;
        else if (proc->state == STOPPED) color = GREEN;
        timesec = (proc->utime + proc->stime) / 100;
        printf("%s%6d %-6d %3ld %2ld %8lu %8ld %8ld %2c %6.1f %6.1f %3.2d:%.2d:%.2d %-5s\e[0m\n",
                color,
                proc->pid,
                proc->uid,
                proc->priority,
                proc->nice,
                proc->virt / (1<<10),
                proc->res,
                proc->shared * (1<<2),
                proc->state,
                (float)  ((proc->utime + proc->stime) / (uptime - (proc->starttime / 100))),
                (float) ((proc->resident + proc->data) * 100) / (mem->memTotal),
                timesec/3600, (timesec/60)%60, timesec%60,
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
    printf("\n");
       
    free(loads);
    free(cpu);
    free(mem);  
    free(proc);  
    alarm(ALARMTIME);
}

//aa
int askAction(struct dirent **list_names,int length) {
    int pid_to_affect, action, trovato = 0, i = 0;

    printf("\b\bEnter the PID of the process that you want to affect or 0 to continue: ");
    scanf("%d", &pid_to_affect);

    if (pid_to_affect == 0) return 0;

    while(i<length) {
        if (pid_to_affect == atoi(list_names[i++]->d_name))
          trovato = 1;
    }

    if (trovato == 0) {
      printf("Invalid PID... retry\n");
      alarm(ALARMTIME);
      print = 0;
      return 0;
    }

    printf("Enter the action - \033[0;35m 0: terminate \033[0;31m 1: kill \033[0;32m 2: suspend \033[0;36m 3: resume\e[0m: ");
    scanf("%d", &action);
    
    switch (action) {
        case 0:
            kill(pid_to_affect, SIGINT);
            printf("Process with PID: %d Interrupted \e[0m\n",pid_to_affect);
            break;
        case 1:
            kill(pid_to_affect, SIGKILL);
            printf("Process with PID: %d Killed \e[0m\n",pid_to_affect);
            break;
        case 2:
            kill(pid_to_affect, SIGSTOP);
            printf("Process with PID: %d Stopped \e[0m\n",pid_to_affect);
            break;
        case 3:
            kill(pid_to_affect, SIGCONT);
            printf("Process with PID: %d Resumed \e[0m\n",pid_to_affect);
            break;
        default:
            printf("Action invalid... retry\n");
            break;
    }

    alarm(ALARMTIME);
    print = 0;
    return 1;
}

int main() {
    signal(SIGALRM, sigalarm_handler); // Register signal handler
    signal(SIGQUIT, sigquit_handler);
    
    while(1) {
        if (print) {
            print_top();            
            print = 0;
        } 
        if (ask) {
            askAction(namelist, len_namelist);
            ask = 0;
        }
    }

    return 0;
}