#include <stdio.h>
#include <signal.h>

void ignore (int sig){
    // do nothing
}
int main() {
    printf("Free Shell started\n");
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
        signal(SIGINT, ignore);
    if (signal(SIGTSTP, SIG_IGN) != SIG_IGN)
        signal(SIGINT, ignore);
    while(1){




    }


    return 0;
}