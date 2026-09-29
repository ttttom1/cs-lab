#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUF_SIZE 1024
#define MAX_CLIENTS 100

// 소켓을 Non-blocking으로 바꿔주는 유틸리티 함수
void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(void) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 5);

    // 1. Listening 소켓도 Non-blocking으로 전환!
    set_nonblocking(server_fd);

    int client_fds[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) client_fds[i] = -1;

    printf("Server running (Polling Mode)...\n");

    while (1) {
        // 2. Non-blocking accept (연결 없으면 바로 -1 / EAGAIN 반환하며 안 잠듦)
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd != -1) {
            printf("[main] new client connected: fd=%d\n", client_fd);
            set_nonblocking(client_fd); // 새 클라이언트 소켓도 Non-blocking 설정

            // 배열의 빈자리에 추가
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_fds[i] == -1) {
                    client_fds[i] = client_fd;
                    break;
                }
            }
        }

        // 3. 모든 클라이언트 소켓을 순회하며 Non-blocking recv (Polling)
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = client_fds[i];
            if (fd == -1) continue;

            char buffer[BUF_SIZE];
            ssize_t n = recv(fd, buffer, sizeof(buffer) - 1, 0);

            if (n > 0) {
                buffer[n] = '\0';
                printf("[fd=%d] received: %s\n", fd, buffer);
            } else if (n == 0) {
                printf("[fd=%d] disconnected\n", fd);
                close(fd);
                client_fds[i] = -1; // 배열에서 제거
            } else {
                // n == -1 이고 errno가 EAGAIN이면 데이터가 없는 것이므로 무시하고 다음 소켓으로 넘어감
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("recv error");
                    close(fd);
                    client_fds[i] = -1;
                }
            }
        }

        // sleep없이 돌리면 CPU 100% 차지함 (Busy Waiting)
        // usleep(1000); 
    }

    close(server_fd);
    return 0;
}