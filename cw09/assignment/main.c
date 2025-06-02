#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int thread_id;
    int num_threads;
    long double dx;
    long long num_intervals;
    long double* results;
} ThreadData;

long double f(long double x) {
    return 4.0L / (x * x + 1.0L);
}

void* thread_function(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int id = data->thread_id;
    int k = data->num_threads;
    long double dx = data->dx;
    long long n = data->num_intervals;
    long double local_sum = 0.0L;

    long long start = id * n / k;
    long long end = (id + 1) * n / k;

    long double x = start * dx;
    for (long long i = start; i < end; ++i) {
        local_sum += f(x) * dx;
        x += dx;
    }

    data->results[id] = local_sum;
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Użycie: %s <dx> [liczba_wątków]\n", argv[0]);
        return 1;
    }

    long double dx = strtold(argv[1], NULL);
    if (dx <= 0.0L || dx > 1.0L) {
        fprintf(stderr, "dx musi być > 0 i <= 1\n");
        return 1;
    }

    int num_threads = (argc == 3) ? atoi(argv[2]) : 4;
    if (num_threads < 1 || num_threads > 1024) {
        fprintf(stderr, "Liczba wątków musi być z przedziału 1–1024\n");
        return 1;
    }

    long long num_intervals = (long long)(1.0L / dx);

    pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
    ThreadData* thread_data = malloc(sizeof(ThreadData) * num_threads);
    long double* results = calloc(num_threads, sizeof(long double));

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_threads;
        thread_data[i].dx = dx;
        thread_data[i].num_intervals = num_intervals;
        thread_data[i].results = results;

        if (pthread_create(&threads[i], NULL, thread_function, &thread_data[i]) != 0) {
            perror("Błąd tworzenia wątku");
            return 1;
        }
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    long double total = 0.0L;
    for (int i = 0; i < num_threads; ++i) {
        total += results[i];
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed = (end_time.tv_sec - start_time.tv_sec) +
                     (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    printf("Wynik całki: %.18Lf\n", total);
    printf("Czas wykonania: %.3f s\n", elapsed);

    free(threads);
    free(thread_data);
    free(results);

    return 0;
}
