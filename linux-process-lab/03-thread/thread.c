#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* worker(void* arg) {
    printf("worker started\n");

    sleep(20);
    
    return NULL;
}

int  main() {
    pthread_t t1;
    pthread_t t2;
    
    pthread_create(&t1, NULL, worker,NULL);
    pthread_create(&t2, NULL, worker, NULL);

    printf("PID = %d\n",getpid());

    pthread_join(t1,NULL);
    pthread_join(t2,NULL);

    return 0;
}