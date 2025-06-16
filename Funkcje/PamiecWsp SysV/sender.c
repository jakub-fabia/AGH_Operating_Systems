#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 1234

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    int value = atoi(argv[1]);

    // Utwórz segment pamięci współdzielonej
    int shmid = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 2;
    }

    // Dołącz segment do przestrzeni adresowej procesu
    int *shm_ptr = (int *)shmat(shmid, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat");
        return 3;
    }

    // Zapisz wartość
    *shm_ptr = value;

    // Odłącz pamięć
    shmdt(shm_ptr);

    return 0;
}
