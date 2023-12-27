#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>

#define SOCKET_PATH "/tmp/socket"

int main() {
    int server_fd, client_fd;
    socklen_t len;
    struct sockaddr_un server_addr, client_addr;
    char buf[1024];

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    unlink(SOCKET_PATH); 

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Сервер ожидает входящие соединения...\n");

    while (1) {
        len = sizeof(struct sockaddr_un);
        if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len)) == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        ssize_t bytes_read;
        while ((bytes_read = read(client_fd, buf, sizeof(buf))) > 0) {
            for (int i = 0; i < bytes_read; i++) {
                buf[i] = (char)toupper(buf[i]);
            }

            write(STDOUT_FILENO, buf, bytes_read);
        }

        close(client_fd);
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}