//
//  main.c
//  Producer-consumer problem
//
//  Created by 서형중 on 29/04/2019.
//  Copyright © 2019 Hyeongjung Seo. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 10

typedef int buffer_item;

buffer_item buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t empty;
sem_t full;

// seed //
unsigned int seed;
unsigned int total_thread_num;


struct multi_arg {
    int threadNum;
    int duration;
};

int produce_item(void){
    // your code
    return rand() % 10000000000;
}

int add_item(buffer_item item){
    // your code
    if(buffer[in] != '\0'){
        // buffer is already full
        return -1;
    }
    else {
        buffer[in] = item;
        in = (in + 1) % 10;
        printf("%s %d\n\n", "produced", item);
        return 0;
    }
}

int remove_item(buffer_item *item){
    // your code
    if (buffer[out] == '\0'){
        //buffer is empty
        return -1;
    }
    else {
        int tmp_item = item[out];
        item[out] = '\0';
        out = (out + 1) % 10;
        return tmp_item;
    }
}

int consume_item(buffer_item item){
    // your code
    
    return !printf("%s %d\n\n", "consumed", item);
}

void *producer(void *param){
    // your code
    struct multi_arg *arg = (struct multi_arg*)param;
    int id = arg->threadNum;
    int duration = arg->duration;
    buffer_item item;
    
    while(1) {
        // sleep(rand_r(&seed) % duration + 1);
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        
        // critical section
        // add the item to the buffer
        item = produce_item();
        printf("P%d          ", id);
        if (add_item(item) == -1) {
            // buffer is already full
            printf("%s\n", "buffer is already full!\n");
        }
        
        // sleep(duration);
        //        sem_post(&full);
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
        sleep(duration%total_thread_num);
    }
    return NULL;
}

void *consumer(void *param){
    // your code
    struct multi_arg *arg = (struct multi_arg*)param;
    int id = arg->threadNum;
    int duration = arg->duration;
    buffer_item item;
    
    while(1) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        // critical section
        // remove an item from buffer
        printf("C%d          ", id);
        if ((item = remove_item(buffer)) == -1){
            // buffer is empty
            printf("%s\n", "buffer is empty1.\n");
        }
        else if (consume_item(item) == -1){
            // buffer is empty
            printf("%s\n", "buffer is empty2.\n");
        }
        
        // sleep(duration);
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
        sleep(duration%total_thread_num + 1);
        // consume the removed item
        
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int duration = 0;
    int number_of_producers = 0;
    int number_of_consumers = 0;
    
    duration = atoi(argv[1]);
    number_of_producers = atoi(argv[2]);
    number_of_consumers = atoi(argv[3]);
    
    // your code
    srand((unsigned int)time(NULL));
    seed = (unsigned int)time(NULL);
    total_thread_num = number_of_producers + number_of_consumers;
    // initialize semaphores
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);
    
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        buffer[i] = '\0';
    }
    // end initialize
    
    
    // set multiple arguments for theads
    struct multi_arg *producer_args, *consumer_args;
    producer_args = (struct multi_arg *)malloc(sizeof(struct multi_arg) * number_of_producers);
    consumer_args = (struct multi_arg *)malloc(sizeof(struct multi_arg) * number_of_consumers);
    for (int i = 0; i < number_of_producers; ++i) {
        producer_args[i].duration = duration;
        producer_args[i].threadNum = i;
    }
    for (int i = 0; i < number_of_consumers; ++i) {
        consumer_args[i].duration = duration;
        consumer_args[i].threadNum = i;
    }
    
    // create a set number of Producers and Consumers
    pthread_t p_threads[number_of_producers], c_threads[number_of_consumers];
    for (int i = 0; i < number_of_producers; ++i) {
        pthread_attr_t p_attr;
        pthread_attr_init(&p_attr);
        int status = pthread_create(&p_threads[i], &p_attr, producer, (void*)&producer_args[i]);
        if(status == -1)
            printf("%s\n", "Error creating producer thread.\n");
    }
    for (int i = 0; i < number_of_consumers; ++i) {
        pthread_attr_t c_attr;
        pthread_attr_init(&c_attr);
        int status = pthread_create(&c_threads[i], &c_attr, consumer, (void*)&consumer_args[i]);
        if(status == -1)
            printf("%s\n", "Error creating producer thread.\n");
    }
    
    sleep(duration);
    
    for(int i = 0; i < number_of_producers; ++i)
        pthread_cancel(p_threads[i]);
    for(int i = 0; i < number_of_consumers; ++i)
        pthread_cancel(c_threads[i]);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
    return 0;
}
