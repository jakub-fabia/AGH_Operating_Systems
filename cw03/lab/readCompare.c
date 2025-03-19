#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void chunk(int chunkSize)
{
    char *buffor = (char *)malloc(chunkSize + 1);
    int descriptor;
    ssize_t bytes_read;
    descriptor = open("text.txt", O_RDONLY);
    while ((bytes_read = read(descriptor, buffor, chunkSize)) > 0) {
        buffor[bytes_read] = '\0';
        printf("%s\n", buffor);
    }
    close(descriptor);
    free(buffor);
}

void oneLetterChunk()
{
  char buffor;
  int descriptor;
  descriptor = open("text.txt", O_RDONLY);
  while(read(descriptor, &buffor, 1) > 0){
    printf("%s", &buffor);
  }
  close(descriptor);
}

void openClose(){
    char buffer;
    int descriptor;
    ssize_t bytes_read;
    int position = 0;
    while (1) {
        descriptor = open("text.txt", O_RDONLY);
        lseek(descriptor, position, SEEK_SET);
        bytes_read = read(descriptor, &buffer, 1);
        if (bytes_read == 0) {
            close(descriptor);
            break;
        }
        printf("%c", buffer);
        fflush(stdout);
        close(descriptor);
        position++;
    }
    printf("\n");
}

#define NUM_CHUNK_SIZES 20

int main() {
    clock_t start, end;
    double oneLetter_time, openClose_time;
    int chunkSizes[NUM_CHUNK_SIZES];
    double chunk_times[NUM_CHUNK_SIZES];

    for (int i = 0; i < NUM_CHUNK_SIZES; i++) {
        chunkSizes[i] = 1 << (i + 5);
    }

    for (int i = 0; i < NUM_CHUNK_SIZES; i++) {
        start = clock();
        chunk(chunkSizes[i]);
        end = clock();
        chunk_times[i] = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    }
    start = clock();
    oneLetterChunk();
    end = clock();
    oneLetter_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

    start = clock();
    openClose();
    end = clock();
    openClose_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;

    printf("\nExecution Times (ms):\n");
    for (int i = 0; i < NUM_CHUNK_SIZES; i++) {
        printf("Chunk function (%d bytes): %.3f ms\n", chunkSizes[i], chunk_times[i]);
    }
    printf("One letter function:        %.3f ms\n", oneLetter_time);
    printf("Open-close function:        %.3f ms\n", openClose_time);

    return 0;
}