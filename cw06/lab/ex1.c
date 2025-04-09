#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MESSAGE_CHILD "echo 'Wiadomosc od dziecka'"

int main() {
    FILE *fp;
    char buffer[128];

    if (fork() == 0) {
        fp = popen(MESSAGE_CHILD, "w");

        pclose(fp);
        exit(EXIT_SUCCESS);
    } else {
        fp = popen("echo $(!!)", "w");
        wait(NULL);
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("Child: %s", buffer);
        }
    }

    return 0;
}