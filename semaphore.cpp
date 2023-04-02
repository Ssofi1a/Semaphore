#include <iostream>
#include <iostream>
#include <cstdlib>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <sys/shm.h>
#include <sys/mman.h>


int *sh_buffer;
sem_t *empty;
sem_t *full;
sem_t *mutex;

void *producer(void* arg) {
    for (int i = 0; i < 100; i++) {
        sem_wait(empty);
        sem_wait(mutex);
        sh_buffer[i % 100] = i;
        std::cout << "Producer generates " << i << "\n";
        sem_post(mutex);
        sem_post(full);
    }
    pthread_exit(0);
}

void *consumer(void* arg) {
    for (int i = 0; i < 100; i++) {
        sem_wait(full);
        sem_wait(mutex);
        int elem = sh_buffer[i % 100];
        std::cout << "Consumer retrived " << elem << "\n";
        sem_post(mutex);
        sem_post(empty);
    }
    pthread_exit(0);
}

int main() {
    
    int shm_fd = shm_open("/shm_buffer", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, 40);
    sh_buffer = (int*) mmap(NULL, 40, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    memset(sh_buffer, 0, 40);
    empty = sem_open("/empty_sem", O_CREAT, 0644, 100);
    full = sem_open("/full_sem", O_CREAT, 0644, 0);
    mutex = sem_open("/mutex_sem", O_CREAT, 0644, 1);
    pthread_t prod_thread, cons_thread;
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
    sem_close(empty);
    sem_close(full);
    sem_close(mutex);
    sem_unlink("/empty_sem");
    sem_unlink("/full_sem");
    sem_unlink("/mutex_sem");
    shm_unlink("/shm_buffer");

    return 0;
}
