#include <signal.h>
#include <unistd.h>
#include <time.h>

#include "read_proc.h"

int print = 1;

void sig_handler(int signum) {
    print = 1;
}


int main() {
    signal(SIGALRM, sig_handler); // Register signal handler
    
    while(1) {
        if(print) {
            print = 0;
            // print top
        } 
    }
}