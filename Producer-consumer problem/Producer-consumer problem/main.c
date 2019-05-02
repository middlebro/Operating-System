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

// ** Count of total threads ** //
unsigned int total_thread_num;

// ** Multiple Argument ** //
struct multi_arg {
    int threadID;
    int duration;
};

int produce_item(void){
    // produce randomly item
    return rand() % 10000000000;
}

int add_item(buffer_item item){
    // test buuffer is available
    if(buffer[in] != '\0'){
        // buffer is already full
        return -1;
    }
    else {
        buffer[in] = item;  // add item to buffer
        in = (in + 1) % 10; // accomodate buffer pointer
        printf("%s %d\n\n", "produced", item);  // show produced item
        return 0;
    }
}

int remove_item(buffer_item *item){
    // test buffer is available
    if (buffer[out] == '\0'){
        //buffer is empty
        return -1;
    }
    else {
        int tmp_item = item[out];   // store item in temporary storage for return
        item[out] = '\0';           // remove item from buffer
        out = (out + 1) % 10;       // accomodate buffer pointer
        return tmp_item;
    }
}

int consume_item(buffer_item item){
    // show consumed item
    return !printf("%s %d\n\n", "consumed", item);
}

void *producer(void *param){
    // cast parameter to struct Multiple Argument
    struct multi_arg *arg = (struct multi_arg*)param;
    int id = arg->threadID;
    int duration = arg->duration;
    buffer_item item;
    while(1) {
        ////////// ** Entry Section ** ////////////////////////////////////////////////////////////////
        sem_wait(&empty);               // empty - 1 and test can be into Critical Section?
        pthread_mutex_lock(&mutex);     // SET Mutex lock to enter Critical Section.
        ////////// ** Critical Section ** /////////////////////////////////////////////////////////////
        item = produce_item();          // produce item
        printf("P%d          ", id);    // show Producer id
        if (add_item(item) == -1) {     // test add item into buffer
            // buffer is already full
            printf("%s\n", "buffer is already full!\n");
        }
        ////////// ** Exit Section ** /////////////////////////////////////////////////////////////////
        pthread_mutex_unlock(&mutex);   // SET Mutex unlock to enter another thread.
        sem_post(&full);                // full + 1 and wakeup thread.
        sleep(duration%total_thread_num); // Sleep for a designated time.
        ///////////////////////////////////////////////////////////////////////////////////////////////
    }
    return NULL;
}

void *consumer(void *param){
    // cast parameter to struct Multiple Argument
    struct multi_arg *arg = (struct multi_arg*)param;
    int id = arg->threadID;
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
    // set seed value
    srand((unsigned int)time(NULL));
    
    // initialize total_thread_num
    total_thread_num = number_of_producers + number_of_consumers;
    // initialize semaphores
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);
    
    // initialize BUFFER
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        buffer[i] = '\0';
    }
    
    // set multiple arguments for theads
    struct multi_arg *producer_args, *consumer_args;
    producer_args = (struct multi_arg *)malloc(sizeof(struct multi_arg) * number_of_producers);
    consumer_args = (struct multi_arg *)malloc(sizeof(struct multi_arg) * number_of_consumers);
    for (int i = 0; i < number_of_producers; ++i) { // initialize argument
        producer_args[i].duration = duration;
        producer_args[i].threadID = i;
    }
    for (int i = 0; i < number_of_consumers; ++i) {
        consumer_args[i].duration = duration;
        consumer_args[i].threadID = i;
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
