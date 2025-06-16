#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/types.h>
#include <pthread.h>

void *hello(void *arg)
{
        int thread_num = *(int *)arg;
        free(arg); // Zwolnij zaalokowaną pamięć

        sleep(1);

        // Włącz obsługę anulowania
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

        while (1)
        {
                printf("Hello world from thread number %d\n", thread_num);

                // Punkt anulowania
                pthread_testcancel();

                printf("Hello again world from thread number %d\n", thread_num);
                sleep(2);
        }

        return NULL;
}

int main(int argc, char **args)
{

        if (argc != 3)
        {
                printf("Not a suitable number of program parameters\n");
                return (1);
        }

        int n = atoi(args[1]);

        pthread_t *threads = malloc(sizeof(pthread_t) * n);

        for (int i = 0; i < n; ++i)
        {
                int *thread_num = malloc(sizeof(int));
                *thread_num = i + 1;
                if (pthread_create(&threads[i], NULL, hello, thread_num) != 0)
                {
                        perror("Failed to create thread");
                        return 2;
                }
        }

        int i = 0;
        while (i++ < atoi(args[2]))
        {
                printf("Hello from main %d\n", i);
                sleep(2);
        }

        i = 0;

        for (int i = 0; i < n; ++i)
        {
                pthread_cancel(threads[i]);     // Żądanie zakończenia
                pthread_join(threads[i], NULL); // Czekanie na zakończenie
        }

        free(threads);

        printf("DONE");

        return 0;
}
