#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int  main() {
    pid_t  pid =fork();

    if (pid == 0) {
        printf("before execpid=%d\n",getpid());

        execl("/bin.ls","ls","-l",NULL);

        printf("this should not print\n");

    } else {
        wait(NULL);

        printf("parent done\n");
    }
    return 0;
}