#include <stdio.h>
#include <unistd.h>

int global = 100;

int main() {

    int local = 200;

    printf("PID = %d\n", getpid());
    printf("global = %p\n", &global);
    printf("local  = %p\n", &local);

    sleep(30);
}