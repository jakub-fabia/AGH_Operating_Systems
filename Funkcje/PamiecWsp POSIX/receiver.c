#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/mman.h>

#define SHM_NAME "/kol_shm"
#define MAX_SIZE 1024

int main(int argc, char **argv)
{

    sleep(1);

    /*******************************************
    Utworz/otworz posixowy obszar pamieci wspolnej "reprezentowany" przez SHM_NAME
    odczytaj zapisana tam wartosc i przypisz ja do zmiennej val
    posprzataj
    *********************************************/
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Zmapuj pamięć do odczytu
    int *shm_ptr = mmap(NULL, sizeof(int), PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        return 2;
    }

    int val = *shm_ptr;

    // Posprzątaj
    munmap(shm_ptr, sizeof(int));
    close(shm_fd);

    // Usuń pamięć współdzieloną
    shm_unlink(SHM_NAME);

	printf("%d square is: %d \n",val, val*val);
    return 0;
}
