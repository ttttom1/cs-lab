#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT  8080
#define MAX_EVENTS 64
#define BUF_SIZE 1024

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1) {
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(1);
    }
    return 0;
}

int main(void) {
    int server_fd, epfd;
    struct sockaddr_in server_addr;
    struct epoll_event ev, events[MAX_EVENTS];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;

    // Fix: SO_REUSEADDR, &opt 오타 수정
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(server_fd);
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Fix: server_addr 변수명 수정
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, SOMAXCONN) == -1) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    set_nonblocking(server_fd);

    // Fix: epfd 중복 선언(int) 제거
    epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl: server_fd");
        close(server_fd);
        close(epfd);
        exit(EXIT_FAILURE);
    }
    printf("[epoll server] listening on port %d....\n", PORT);

    while (1) {
        int ready = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (ready == -1) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < ready; i++) {
            int active_fd = events[i].data.fd;

            if (active_fd == server_fd) {
                // Fix: accept 루프 내에서 신규 클라이언트 처리 완료하도록 구조 변경
                while (1) {
                    int client_fd = accept(server_fd, NULL, NULL);
                    if (client_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break; // 더 이상 수락할 접속이 없음 (정상 탈출)
                        }
                        perror("accept");
                        break;
                    }

                    printf("[main] new client connected: fd=%d\n", client_fd);

                    set_nonblocking(client_fd);

                    struct epoll_event client_ev;
                    client_ev.events = EPOLLIN;
                    client_ev.data.fd = client_fd;
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &client_ev) == -1) {
                        perror("epoll_ctl: client_fd");
                        close(client_fd);
                    }
                }
            }
            else {
                char buffer[BUF_SIZE];

                while (1) {
                    ssize_t n = recv(active_fd, buffer, sizeof(buffer) - 1, 0);

                    if (n > 0) {
                        buffer[n] = '\0';
                        // Fix: %%s -> %s 로 수정
                        printf("[fd=%d] recv: %s", active_fd, buffer);
                        send(active_fd, buffer, n, 0);
                    }
                    else if (n == 0) {
                        printf("[fd=%d] client disconnected\n", active_fd);

                        epoll_ctl(epfd, EPOLL_CTL_DEL, active_fd, NULL);
                        close(active_fd);
                        break;
                    }
                    else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break; // 커널 수신 버퍼 비었음 (정상 탈출)
                        }
                        perror("recv error");
                        epoll_ctl(epfd, EPOLL_CTL_DEL, active_fd, NULL);
                        close(active_fd);
                        break;
                    }
                }
            }
        }
    }

    close(server_fd);
    close(epfd);
    return 0;
}