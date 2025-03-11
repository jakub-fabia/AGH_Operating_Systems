#include <stdio.h>

void printMessage(){
    printf("Hello World\n");
}

void welcome(){
    char name[15];
    printf("What's your name?\n");
    scanf("%s", name);
    printf("Welcome %s\n", name);
}