#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/socket" // Путь к сокету

int main() {
    int client_fd;
    struct sockaddr_un server_addr;
    char input_buffer[1024];

    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    do {
        printf("Введите текст для отправки на сервер (для завершения введите 'exit'): ");
        fgets(input_buffer, sizeof(input_buffer), stdin);
        write(client_fd, input_buffer, strlen(input_buffer));
    } while (strncmp(input_buffer, "exit", 4) != 0); 

    close(client_fd);

    return 0;
}
