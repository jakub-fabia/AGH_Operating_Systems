#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 1234

int main(int argc, char **argv)
{
    sleep(1);  // Daj czas senderowi na zapisanie wartości

    // Uzyskaj dostęp do segmentu pamięci
    int shmid = shmget(SHM_KEY, sizeof(int), 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    // Dołącz segment do przestrzeni adresowej procesu
    int *shm_ptr = (int *)shmat(shmid, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat");
        return 2;
    }

    int val = *shm_ptr;

    // Odłącz pamięć
    shmdt(shm_ptr);

    // Usuń segment pamięci
    shmctl(shmid, IPC_RMID, NULL);

    printf("%d square is: %d \n", val, val * val);
    return 0;
}
