#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main()
{
    char buffor;
    int descriptor;
    descriptor = open("tekst.txt", O_RDONLY);
    lseek(descriptor, descriptor, SEEK_CUR);
    if (read(descriptor, &buffor, 1) > 0){
        printf("%s\n", &buffor);
    }
    close(descriptor);
}