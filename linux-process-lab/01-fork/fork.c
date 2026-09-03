#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    printf("parent start pid=%%d\n", getpid());

    pid_t pid =  fork();

    if (pid == 0) {
        printf("child pid=%d, ppid=%d\n", getpid(), getppid());
        sleep(10);
    } else {
        printf("parent pid=%d, child=%d\n",getpid(), pid);

        wait(NULL);
        
        printf("child finished\n");
    }
    return 0;
}