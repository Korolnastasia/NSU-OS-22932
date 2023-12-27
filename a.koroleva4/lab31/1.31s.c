#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/select.h>

#define SOCKET_PATH "/tmp/socket" // Путь к сокету

int main() {
    int server_fd, client_fd, new_fd, max_fd;
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

    fd_set read_fds, master_fds;
    FD_ZERO(&master_fds);
    FD_SET(server_fd, &master_fds);
    max_fd = server_fd;

    while (1) {
        read_fds = master_fds; // копируем набор дескрипторов для передачи в select

        // Используем select для мультиплексирования наших соединений
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // Перебираем все сокеты в нашем множестве
        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == server_fd) {
                    // Новое входящее соединение
                    len = sizeof(struct sockaddr_un);
                    if ((new_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len)) == -1) {
                        perror("accept");
                    }
                    else {
                        FD_SET(new_fd, &master_fds);
                        if (new_fd > max_fd) {
                            max_fd = new_fd;
                        }

                        printf("Новое соединение установлено\n");
                    }
                }
                else {
                    // Обработка клиентского сокета
                    ssize_t bytes_read = read(i, buf, sizeof(buf));

                    if (bytes_read <= 0) {
                        // Соединение было закрыто или произошла ошибка, удаляем его из набора
                        close(i);
                        FD_CLR(i, &master_fds);
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
    }

    return 0;
}
