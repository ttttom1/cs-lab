#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int data = 0;
int has_data = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* producer(void* arg) {
    
    sleep(2);
    pthread_mutex_lock(&mutex);

    data = 100;
    has_data = 1;

    printf("[Producer] data produced: %d\n",data);

    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&mutex);

    return NULL;
}

void* consumer(void* arg) {
    
    pthread_mutex_lock(&mutex);

    while (has_data == 0) {

        printf("[Consumer] no data. waiting...\n");
        
        pthread_cond_wait(&cond, &mutex);
    }

    printf("[Consumer] data received: %d\n", data);

    has_data = 0;

    pthread_mutex_unlock(&mutex);

    return NULL;
}


int main() {
    pthread_t producer_thread;
    pthread_t consumer_thread;

    pthread_create(&consumer_thread, NULL, consumer, NULL);
    pthread_create(&producer_thread, NULL, producer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    return 0;
}