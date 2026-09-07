#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int count = 0;

void* worker(void* arg) {
    int local = 10;

    count++;
    printf("[Thread %ld] global count = %d, address =  %p\n",(long)arg,count,&count);

    printf("[Thread %ld] local address = %p\n",(long)arg, &local);
    

    sleep(30);
    return NULL;
}

int  main() {
    pthread_t t1;
    pthread_t t2;
    
    pthread_create(&t1, NULL, worker, (void*)1);
    pthread_create(&t2, NULL, worker, (void*)2);

    printf("PID = %d\n",getpid());

    pthread_join(t1,NULL);
    pthread_join(t2,NULL);

    return 0;
}