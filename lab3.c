#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

sem_t s;
int producer_count = 0;
int consumer_count = 0;

void *producer(void *arg) {
    int fd = *((int *)arg);
    for (int i = 0; i < 1000; i++) {
        sem_wait(&s);
        char buf[20];
        sprintf(buf, "produce %d\n", producer_count++);
        write(fd, buf, strlen(buf));
        sem_post(&s);
    }
    return NULL;
}

void *consumer(void *arg) {
    int fd = *((int *)arg);
    while(1) {
        sem_wait(&s);
        if (consumer_count < producer_count) {
            char buf[20];
            sprintf(buf, "consumer %d\n", consumer_count++);
            write(fd, buf, strlen(buf));
        } else if (consumer_count == 1000){
            sem_post(&s);
            break;
        }
        sem_post(&s);
    }
    return NULL;
}
int main() {
    sem_init(&s, 0, 1);
    int fd = open("log.txt", O_CREAT | O_RDWR, 0755);
    assert(fd > 0);
    pthread_t p1, p2;
    int rc = pthread_create(&p1, NULL, producer, (void *)&fd);
    assert(rc == 0);
    rc = pthread_create(&p2, NULL, consumer, (void *)&fd);
    assert(rc == 0);
    rc = pthread_join(p1, NULL);
    assert(rc == 0);
    rc = pthread_join(p2, NULL);
    assert(rc == 0);
}