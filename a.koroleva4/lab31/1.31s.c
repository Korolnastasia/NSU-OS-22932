#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/select.h>

#define SOCKET_PATH "/tmp/socket" // ���� � ������

int main() {
    int server_fd, client_fd, new_fd, max_fd;
    socklen_t len;
    struct sockaddr_un server_addr, client_addr;
    char buf[1024];

    // �������� ������
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // ��������� ���������� ������ �������
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    unlink(SOCKET_PATH); // ������� ������ �����, ���� �� ����������

    // �������� ������ � ������
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // �������� �������� ����������
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("������ ������� �������� ����������...\n");

    fd_set read_fds, master_fds;
    FD_ZERO(&master_fds);
    FD_SET(server_fd, &master_fds);
    max_fd = server_fd;

    while (1) {
        read_fds = master_fds; // �������� ����� ������������ ��� �������� � select

        // ���������� select ��� ������������������� ����� ����������
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // ���������� ��� ������ � ����� ���������
        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == server_fd) {
                    // ����� �������� ����������
                    len = sizeof(struct sockaddr_un);
                    if ((new_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len)) == -1) {
                        perror("accept");
                    }
                    else {
                        FD_SET(new_fd, &master_fds);
                        if (new_fd > max_fd) {
                            max_fd = new_fd;
                        }

                        printf("����� ���������� �����������\n");
                    }
                }
                else {
                    // ��������� ����������� ������
                    ssize_t bytes_read = read(i, buf, sizeof(buf));

                    if (bytes_read <= 0) {
                        // ���������� ���� ������� ��� ��������� ������, ������� ��� �� ������
                        close(i);
                        FD_CLR(i, &master_fds);
                    }
                    else {
                        // �������������� ������ � ������� ������� � ����� � stdout
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
