#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>
#include <sys/wait.h>

#define N_USERS 3
#define M_PRINTERS 2
#define QUEUE_SIZE 7
#define JOB_LENGTH 10

typedef struct {
    char jobs[QUEUE_SIZE][JOB_LENGTH + 1];
    int head;
    int tail;
} JobQueue;

sem_t *sem_mutex;
sem_t *sem_full;
sem_t *sem_empty;

JobQueue *queue;

void enqueue_job(const char *job) {
    sem_wait(sem_empty);
    sem_wait(sem_mutex);

    strcpy(queue->jobs[queue->tail], job);
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;

    sem_post(sem_mutex);
    sem_post(sem_full);
}

void dequeue_job(char *job_buf) {
    sem_wait(sem_full);
    sem_wait(sem_mutex);

    strcpy(job_buf, queue->jobs[queue->head]);
    queue->head = (queue->head + 1) % QUEUE_SIZE;

    sem_post(sem_mutex);
    sem_post(sem_empty);
}

void user_process(int id) {
    srand(time(NULL) ^ (getpid() << 16));
    while (1) {
        char job[JOB_LENGTH + 1];
        for (int i = 0; i < JOB_LENGTH; ++i) {
            job[i] = 'a' + rand() % 26;
        }
        job[JOB_LENGTH] = '\0';

        printf("User %d: sends task \"%s\"\n", id, job);
        enqueue_job(job);

        sleep(1 + rand() % 3);
    }
}

void printer_process(int id) {
    while (1) {
        char job[JOB_LENGTH + 1];
        dequeue_job(job);

        printf("Printer %d: starts printing \"%s\"\n", id, job);
        for (int i = 0; i < JOB_LENGTH; ++i) {
            printf("Printer %d: %c\n", id, job[i]);
            fflush(stdout);
            sleep(1);
        }
        printf("Printer %d: finished printing\n", id);
    }
}

int main() {
    int shm_fd = shm_open("/job_queue", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(JobQueue));
    queue = mmap(NULL, sizeof(JobQueue), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    queue->head = queue->tail = 0;

    sem_mutex = sem_open("/sem_mutex", O_CREAT, 0666, 1);
    sem_full = sem_open("/sem_full", O_CREAT, 0666, 0);
    sem_empty = sem_open("/sem_empty", O_CREAT, 0666, QUEUE_SIZE);

    for (int i = 0; i < N_USERS; ++i) {
        if (fork() == 0) {
            user_process(i + 1);
            exit(0);
        }
    }

    for (int i = 0; i < M_PRINTERS; ++i) {
        if (fork() == 0) {
            printer_process(i + 1);
            exit(0);
        }
    }

    for (int i = 0; i < N_USERS + M_PRINTERS; ++i) {
        wait(NULL);
    }

    sem_close(sem_mutex);
    sem_close(sem_full);
    sem_close(sem_empty);
    sem_unlink("/sem_mutex");
    sem_unlink("/sem_full");
    sem_unlink("/sem_empty");
    shm_unlink("/job_queue");

    return 0;
}
