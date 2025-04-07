#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int global = 0;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <directory_path>\n", argv[0]);
        return 1;
    }
    
    int local = 0;
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        printf("child process\n");
        global++;
        local++;
        printf("child pid = %d, parent pid = %d\n", (int)getpid(), (int)getppid());
        printf("child's local = %d, child's global = %d\n", local, global);
        execl("/bin/ls", "ls", argv[1], (char *)NULL);
        perror("execl failed");
        exit(1);
    } else {
        int status;
        wait(&status);
        printf("parent process\n");
        printf("parent pid = %d, child pid = %d\n", (int)getpid(), (int)pid);
        printf("Child exit code: %d\n", WEXITSTATUS(status));
        printf("Parent's local = %d, parent's global = %d\n", local, global);
        return WEXITSTATUS(status);
    }
}