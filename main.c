#include <signal.h>
#include <unistd.h>
#include <time.h>

#include "read_proc.h"

int print = 1;

void sig_handler(int signum) {
    print = 1;
}


/* Useful links:
 * https://linuxaria.com/howto/understanding-the-top-command-on-linux?lang=it */
void print_top() {
    write(STDOUT_FILENO, "\e[1;1H\e[2J", 11); 

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