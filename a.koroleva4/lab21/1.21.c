#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>

struct one saved;
int count = 0;

void sound() {
    printf("\a");
    fflush(NULL);
    ++count;
}

void end() {
    printf("There were %d sounds\n", count);
    tcsetattr(0, TCSANOW, &saved);
    exit(0);
}

int main() {
    struct one t;
    tcgetattr(0, &t);
    memcpy(&saved, &t, sizeof(t));
    t.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &t);
    sigset(SIGINT, sound);
    sigset(SIGQUIT, end);
    while (1);
}