#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define APTEKA_CAPACITY 6

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_doctor            = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_patient           = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_not_full          = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_pharmacist_queue  = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_pharmacist_done   = PTHREAD_COND_INITIALIZER;

int waiting_patients        = 0;
int waiting_patient_ids[3];
int meds                    = APTEKA_CAPACITY;
int waiting_pharmacists     = 0;
int patients_treated        = 0;
int total_patients          = 0;
int total_pharmacists       = 0;
int done_flag               = 0;

int pharmacist_served       = 0;
int pharmacist_done         = 0;

void current_time_str(char *buf, size_t bufsize) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, bufsize, "%H:%M:%S", tm_info);
}

void* doctor_thread(void* arg) {
    (void)arg;
    while (1) {
        pthread_mutex_lock(&mutex);
                if (patients_treated >= total_patients) {
                        done_flag = 1;
            pthread_cond_broadcast(&cond_not_full);
            pthread_cond_broadcast(&cond_pharmacist_queue);
            pthread_mutex_unlock(&mutex);
            break;
        }
                        while (!((waiting_patients >= 3 && meds >= 3) ||
                 (waiting_pharmacists > 0 && meds < APTEKA_CAPACITY))) {
            pthread_cond_wait(&cond_doctor, &mutex);
                        if (patients_treated >= total_patients) {
                done_flag = 1;
                pthread_cond_broadcast(&cond_not_full);
                pthread_cond_broadcast(&cond_pharmacist_queue);
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
        }
                {
            char tbuf[9];
            current_time_str(tbuf, sizeof(tbuf));
            printf("[%s] - Lekarz: budzę się\n", tbuf);
            fflush(stdout);
        }
                if (waiting_patients >= 3 && meds >= 3) {
                        int ids[3];
            for (int i = 0; i < 3; i++) {
                ids[i] = waiting_patient_ids[i];
            }
                        waiting_patients = 0;
                        meds -= 3;
                        patients_treated += 3;
                        pthread_cond_signal(&cond_not_full);
                        {
                char tbuf[9];
                current_time_str(tbuf, sizeof(tbuf));
                printf("[%s] - Lekarz: konsultuję pacjentów %d, %d, %d\n",
                       tbuf, ids[0], ids[1], ids[2]);
                fflush(stdout);
            }
                        pthread_mutex_unlock(&mutex);
                        int duration = (rand() % 3) + 2;             sleep(duration);
                        pthread_mutex_lock(&mutex);
                        pthread_cond_broadcast(&cond_patient);
                        {
                char tbuf[9];
                current_time_str(tbuf, sizeof(tbuf));
                printf("[%s] - Lekarz: zasypiam.\n", tbuf);
                fflush(stdout);
            }
            pthread_mutex_unlock(&mutex);
        }
                else if (waiting_pharmacists > 0 && meds < APTEKA_CAPACITY) {
                        {
                char tbuf[9];
                current_time_str(tbuf, sizeof(tbuf));
                printf("[%s] - Lekarz: przyjmuję dostawę leków\n", tbuf);
                fflush(stdout);
            }
                        pharmacist_served = 1;
            pthread_cond_signal(&cond_pharmacist_queue);
                        pthread_mutex_unlock(&mutex);
            int duration = (rand() % 3) + 1;             sleep(duration);
                        pthread_mutex_lock(&mutex);
            meds = APTEKA_CAPACITY;
                        pharmacist_done = 1;
            pthread_cond_signal(&cond_pharmacist_done);
                        {
                char tbuf[9];
                current_time_str(tbuf, sizeof(tbuf));
                printf("[%s] - Lekarz: zasypiam.\n", tbuf);
                fflush(stdout);
            }
            pthread_mutex_unlock(&mutex);
        }
        else {
                        pthread_mutex_unlock(&mutex);
        }
    }
    return NULL;
}

void* patient_thread(void* arg) {
    int id = *((int*)arg);
    free(arg);

        while (1) {
                int travel = (rand() % 4) + 2;         {
            char tbuf[9];
            current_time_str(tbuf, sizeof(tbuf));
            printf("[%s] - Pacjent(%d): Idę do szpitala, będę za %d s.\n",
                   tbuf, id, travel);
            fflush(stdout);
        }
        sleep(travel);

        pthread_mutex_lock(&mutex);
                if (waiting_patients == 3) {
            pthread_mutex_unlock(&mutex);
            int roam = (rand() % 4) + 2;             {
                char tbuf[9];
                current_time_str(tbuf, sizeof(tbuf));
                printf("[%s] - Pacjent(%d): Za dużo pacjentów, wracam później za %d s.\n",
                       tbuf, id, roam);
                fflush(stdout);
            }
            sleep(roam);
            continue;
        }
                waiting_patient_ids[waiting_patients] = id;
        waiting_patients++;
        {
            char tbuf[9];
            current_time_str(tbuf, sizeof(tbuf));
            printf("[%s] - Pacjent(%d): Czeka %d pacjentów na lekarza.\n",
                   tbuf, id, waiting_patients);
            fflush(stdout);
        }
                if (waiting_patients == 3) {
            char tbuf[9];
            current_time_str(tbuf, sizeof(tbuf));
            printf("[%s] - Pacjent(%d): Budzę lekarza.\n", tbuf, id);
            fflush(stdout);
            pthread_cond_signal(&cond_doctor);
        }
                while (1) {
            pthread_cond_wait(&cond_patient, &mutex);
                        break;
        }
        pthread_mutex_unlock(&mutex);
                {
            char tbuf[9];
            current_time_str(tbuf, sizeof(tbuf));
            printf("[%s] - Pacjent(%d): Kończę wizytę.\n", tbuf, id);
            fflush(stdout);
        }
        break;
    }
    return NULL;
}

void* pharmacist_thread(void* arg) {
    int id = *((int*)arg);
    free(arg);

        int travel = (rand() % 11) + 5;     {
        char tbuf[9];
        current_time_str(tbuf, sizeof(tbuf));
        printf("[%s] - Farmaceuta(%d): Idę do szpitala, będę za %d s.\n",
               tbuf, id, travel);
        fflush(stdout);
    }
    sleep(travel);

    pthread_mutex_lock(&mutex);
        while (meds == APTEKA_CAPACITY && !done_flag) {
        char tbuf[9];
        current_time_str(tbuf, sizeof(tbuf));
        printf("[%s] - Farmaceuta(%d): Czekam na opróżnienie apteczki.\n",
               tbuf, id);
        fflush(stdout);
        pthread_cond_wait(&cond_not_full, &mutex);
    }
    if (done_flag) {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
        if (meds < 3) {
                {
            char tbuf[9];
            current_time_str(tbuf, sizeof(tbuf));
            printf("[%s] - Farmaceuta(%d): Budzę lekarza.\n", tbuf, id);
            fflush(stdout);
        }
        waiting_pharmacists++;
        pthread_cond_signal(&cond_doctor);
                while (!pharmacist_served && !done_flag) {
            pthread_cond_wait(&cond_pharmacist_queue, &mutex);
        }
        if (done_flag) {
                        pthread_mutex_unlock(&mutex);
            return NULL;
        }
                pharmacist_served = 0;
        waiting_pharmacists--;
                {
            char tbuf[9];
            current_time_str(tbuf, sizeof(tbuf));
            printf("[%s] - Farmaceuta(%d): Dostarczam leki.\n", tbuf, id);
            fflush(stdout);
        }
                while (!pharmacist_done && !done_flag) {
            pthread_cond_wait(&cond_pharmacist_done, &mutex);
        }
        if (done_flag) {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        pharmacist_done = 0;
                {
            char tbuf[9];
            current_time_str(tbuf, sizeof(tbuf));
            printf("[%s] - Farmaceuta(%d): Zakończyłem dostawę.\n", tbuf, id);
            fflush(stdout);
        }
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <liczba_pacjentów> <liczba_farmaceutów>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    total_patients = atoi(argv[1]);
    total_pharmacists = atoi(argv[2]);
    if (total_patients <= 0 || total_pharmacists <= 0) {
        fprintf(stderr, "Parametry muszą być dodatnie.\n");
        exit(EXIT_FAILURE);
    }
        if (total_patients % 3 != 0) {
        fprintf(stderr, "Uwaga: liczba pacjentów nie jest wielokrotnością 3.\n"
                        "Ostatnia grupa (<3) nie zostanie skonsultowana.\n");
    }
    srand(time(NULL));

    pthread_t doctor_tid;
    pthread_t *patients_tids = malloc(sizeof(pthread_t) * total_patients);
    pthread_t *pharmacists_tids = malloc(sizeof(pthread_t) * total_pharmacists);

        if (pthread_create(&doctor_tid, NULL, doctor_thread, NULL) != 0) {
        perror("pthread_create doctor");
        exit(EXIT_FAILURE);
    }
        for (int i = 0; i < total_patients; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        if (pthread_create(&patients_tids[i], NULL, patient_thread, id) != 0) {
            perror("pthread_create patient");
            exit(EXIT_FAILURE);
        }
    }
        for (int i = 0; i < total_pharmacists; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        if (pthread_create(&pharmacists_tids[i], NULL, pharmacist_thread, id) != 0) {
            perror("pthread_create pharmacist");
            exit(EXIT_FAILURE);
        }
    }
        for (int i = 0; i < total_patients; i++) {
        pthread_join(patients_tids[i], NULL);
    }
        pthread_join(doctor_tid, NULL);
        for (int i = 0; i < total_pharmacists; i++) {
        pthread_join(pharmacists_tids[i], NULL);
    }
    free(patients_tids);
    free(pharmacists_tids);

    return 0;
}