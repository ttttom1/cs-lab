#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_CLIENTS 100
#define BUF_SIZE 1024

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(1);
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(1);
    }
}

int main(void) {

    // 1. Listening Socket 생성
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    int opt = 1;

    if (setsockopt(
            server_fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &opt,
            sizeof(opt)
        ) == -1) {

        perror("setsockopt");
        close(server_fd);
        exit(1);
    }

    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(
            server_fd,
            (struct sockaddr *)&addr,
            sizeof(addr)
        ) == -1) {

        perror("bind");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    // poll 기반에서도 non-blocking으로 사용
    set_nonblocking(server_fd);


    // 2. poll이 감시할 FD 배열
    struct pollfd fds[MAX_CLIENTS + 1];

    // 사용하지 않는 칸은 fd = -1
    for (int i = 0; i < MAX_CLIENTS + 1; i++) {
        fds[i].fd = -1;
    }


    // 3. 0번 칸은 Listening Socket
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    printf("poll server running on port %d\n", PORT);


    while (1) {

        // 4. 준비된 FD가 생길 때까지 대기
        int ready = poll(
            fds,
            MAX_CLIENTS + 1,
            -1
        );

        if (ready == -1) {

            if (errno == EINTR) {
                continue;
            }

            perror("poll");
            break;
        }


        // 5. server_fd에 이벤트 발생
        // = 새로운 연결이 있을 가능성
        if (fds[0].revents & POLLIN) {

            while (1) {

                int client_fd =
                    accept(server_fd, NULL, NULL);

                if (client_fd == -1) {

                    if (errno == EAGAIN ||
                        errno == EWOULDBLOCK) {

                        break;
                    }

                    perror("accept");
                    break;
                }

                printf(
                    "[main] new client: fd=%d\n",
                    client_fd
                );

                set_nonblocking(client_fd);


                // 빈 pollfd 자리에 client 저장
                int added = 0;

                for (int i = 1;
                     i < MAX_CLIENTS + 1;
                     i++) {

                    if (fds[i].fd == -1) {

                        fds[i].fd = client_fd;
                        fds[i].events = POLLIN;

                        added = 1;
                        break;
                    }
                }

                if (!added) {
                    printf("too many clients\n");
                    close(client_fd);
                }
            }
        }


        // 6. Client FD 이벤트 처리
        for (int i = 1;i < MAX_CLIENTS + 1;i++) {

            int fd = fds[i].fd;
            if (fd == -1) {
                continue;
            }


            // 읽을 수 있는 상태
            if (fds[i].revents & POLLIN) {
                //소켓의 버퍼와는 다른 버퍼, 가져오는  것
                char buffer[BUF_SIZE];

                ssize_t n = recv(
                    fd,
                    buffer,
                    sizeof(buffer) - 1,
                    0
                );


                if (n > 0) {

                    buffer[n] = '\0';

                    printf(
                        "[fd=%d] received: %s\n",
                        fd,
                        buffer
                    );

                    const char *response = "echo from poll server";

                    send(
                        fd,
                        response,
                        strlen(response),
                        0
                    );

                } else if (n == 0) {

                    printf(
                        "[fd=%d] disconnected\n",
                        fd
                    );

                    close(fd);
                    fds[i].fd = -1;

                } else {

                    if (errno != EAGAIN &&
                        errno != EWOULDBLOCK) {

                        perror("recv");

                        close(fd);
                        fds[i].fd = -1;
                    }
                }
            }


            // 상대 종료 / socket 오류 등
            if (fds[i].revents &
                (POLLERR | POLLHUP | POLLNVAL)) {

                printf(
                    "[fd=%d] socket closed/error\n",
                    fd
                );

                close(fd);
                fds[i].fd = -1;
            }
        }
    }

    //① '불시의 사고' 또는 예외 발생 시의 자원 회수 (Safety Net)
    for (int i = 1; i < MAX_CLIENTS + 1; i++) {

        if (fds[i].fd != -1) {
            close(fds[i].fd);
        }
    }

    close(server_fd);

    return 0;
}