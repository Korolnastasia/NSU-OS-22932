#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    int fd[2];
    pid_t pid;

    if (pipe(fd) == -1) {
        fprintf(stderr, "Pipe failed");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed");
        return 1;
    }

    if (pid > 0) { 
        close(fd[0]); 

        char text[] = "Привет, друг! Как дела?"; 
        write(fd[1], text, strlen(text) + 1); 

        close(fd[1]); 
        wait(NULL);    
    }
    else { 
        close(fd[1]); 

        char received_text[100];
        read(fd[0], received_text, sizeof(received_text)); 

        for (int i = 0; received_text[i]; i++) {
            received_text[i] = toupper(received_text[i]);  
        }

        printf("%s\n", received_text); 

        close(fd[0]); 
    }

    return 0;
}