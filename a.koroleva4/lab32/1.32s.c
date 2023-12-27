#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define SOCKET_PATH "/tmp/socket" // Путь к сокету

int main() {
    int server_fd, client_fd;
    socklen_t len;
    struct sockaddr_un server_addr, client_addr;
    char buf[1024];

    // Создание сокета
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Установка параметров адреса сервера
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    unlink(SOCKET_PATH); // Удаляем старый сокет, если он существует

    // Привязка адреса к сокету
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Ожидание входящих соединений
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Сервер ожидает входящие соединения...\n");

    struct epoll_event event, events[MAX_EVENTS];
    int epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    event.events = EPOLLIN;
    event.data.fd = server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        perror("epoll_ctl: server_fd");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_fd) {
                // Новое соединение
                len = sizeof(struct sockaddr_un);
                if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len)) == -1) {
                    perror("accept");
                }
                else {
                    event.events = EPOLLIN;
                    event.data.fd = client_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                        perror("epoll_ctl: client_fd");
                        exit(EXIT_FAILURE);
                    }

                    printf("Новое соединение установлено\n");
                }
            }
            else {
                // Чтение данных от клиента и преобразование в верхний регистр
                ssize_t bytes_read = read(events[i].data.fd, buf, sizeof(buf));

                if (bytes_read <= 0) {
                    // Соединение было закрыто или произошла ошибка, удаляем его из epoll
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);
                }
                else {
                    // Преобразование текста в верхний регистр и вывод в stdout
                    for (int j = 0; j < bytes_read; j++) {
                        buf[j] = (char)toupper(buf[j]);
                    }
                    write(STDOUT_FILENO, buf, bytes_read);
                }
            }
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
