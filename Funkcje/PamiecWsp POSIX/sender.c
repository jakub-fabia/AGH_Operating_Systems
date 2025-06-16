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
    
   if(argc !=2){
    printf("Not a suitable number of program parameters\n");
    return(1);
   }

   /***************************************
   Utworz/otworz posixowy obszar pamieci wspolnej "reprezentowany" przez SHM_NAME
   zapisz tam wartosc przekazana jako parametr wywolania programu
   posprzataj
   *****************************************/

   int value = atoi(argv[1]);

    // Utwórz/otwórz pamięć współdzieloną
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 2;
    }

    // Ustaw rozmiar pamięci
    if (ftruncate(shm_fd, sizeof(int)) == -1) {
        perror("ftruncate");
        return 3;
    }

    // Zmapuj pamięć
    int *shm_ptr = mmap(NULL, sizeof(int), PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        return 4;
    }

    // Zapisz wartość
    *shm_ptr = value;

    // Posprzątaj
    munmap(shm_ptr, sizeof(int));
    close(shm_fd);
 
    return 0;
}
