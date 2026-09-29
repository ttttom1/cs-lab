#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>

#define PORT 8080
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

// 소켓을 Non-blocking 모드로 전환하는 함수
void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL 실패");
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL O_NONBLOCK 실패");
    }
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // 1. IPv4 TCP 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    // 소켓 재사용 옵션 설정 (서버 재시작 시 PORT 이미 사용 중 에러 방지)
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // ★ [중요] 리스닝 소켓을 Non-blocking으로 설정
    // accept() 호출 시 대기 큐가 비어있어도 스레드가 멈추지 않고 즉시 에러(EWOULDBLOCK) 반환
    set_nonblocking(server_fd);

    // 2. IP 및 Port 바인딩
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 모든 Network Interface에서 수신
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("바인딩 실패");
        exit(EXIT_FAILURE);
    }

    // 3. 리스닝 모드 전환 (연결 대기 큐 생성)
    if (listen(server_fd, 10) < 0) {
        perror("listen 실패");
        exit(EXIT_FAILURE);
    }

    printf("서버가 %d 포트에서 수신 대기 중입니다...\n", PORT);

    // 4. pollfd 배열 관리 구조체 선언 및 초기화
    // Index 0: 리스닝 소켓 전용 / Index 1 ~ MAX_CLIENTS: 클라이언트 통신 소켓
    struct pollfd fds[MAX_CLIENTS + 1];

    // 배열 전체 초기화 (fd = -1이면 poll()이 감시 대상에서 제외함)
    for (int i = 0; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;
    }

    // [Index 0] 리스닝 소켓 감시 등록 (대기 큐에 접속 요청이 오는지 감시)
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    // 5. 메인 이벤트 루프
    while (1) {
        // 커널에게 이벤트 감시 요청 (타임아웃 -1: 이벤트 발생 전까지 대기)
        int ready = poll(fds, MAX_CLIENTS + 1, -1);

        if (ready < 0) {
            perror("poll 실패");
            break;
        }

        // -------------------------------------------------------------
        // Case A. 리스닝 소켓(fds[0])에 POLLIN 이벤트 발생 (새 클라이언트 접속 요청)
        // -------------------------------------------------------------
        if (fds[0].revents & POLLIN) {
            // 대기 큐에서 연결 요청을 꺼내 클라이언트 전용 소켓(client_fd) 생성
            int client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);

            if (client_fd < 0) {
                // Non-blocking 소켓에서 처리할 데이터가 없거나 인터럽트된 경우는 예외 처리
                if (errno != EWOULDBLOCK && errno != EAGAIN) {
                    perror("accept 실패");
                }
            } else {
                // ★ [중요] 새로 연결된 클라이언트 통신 소켓도 Non-blocking으로 전환
                set_nonblocking(client_fd);

                // fds 배열의 빈자리(fd == -1)를 찾아 클라이언트 등록
                int added = 0;
                for (int i = 1; i <= MAX_CLIENTS; i++) {
                    if (fds[i].fd == -1) {
                        fds[i].fd = client_fd;
                        fds[i].events = POLLIN; // 상대방 데이터 수신(POLLIN) 감시 설정
                        printf("[접속] 클라이언트 추가됨 (fd: %d, index: %d)\n", client_fd, i);
                        added = 1;
                        break;
                    }
                }

                // 최대 접속 수를 초과한 경우 접속 거절 후 닫기
                if (!added) {
                    printf("[거절] 최대 클라이언트 수 초과 (fd: %d)\n", client_fd);
                    close(client_fd);
                }
            }
        }

        // -------------------------------------------------------------
        // Case B. 기존 클라이언트 소켓(fds[1 ~ MAX])에 이벤트 발생 (데이터 수신 또는 종료)
        // -------------------------------------------------------------
        for (int i = 1; i <= MAX_CLIENTS; i++) {
            // 빈 슬롯은 건너뜀
            if (fds[i].fd == -1) continue;

            // 해당 클라이언트 소켓에서 읽을거리(POLLIN)가 생긴 경우
            if (fds[i].revents & POLLIN) {
                char buffer[BUFFER_SIZE] = {0};

                // Non-blocking 모드이므로 수신 버퍼에 있는 만큼만 읽고 즉시 반환됨
                int bytes_read = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);

                if (bytes_read > 0) {
                    // [정상 수신] 에코(Echo): 받은 메시지를 클라이언트에게 다시 전송
                    printf("[수신 fd: %d]: %s", fds[i].fd, buffer);
                    send(fds[i].fd, buffer, bytes_read, 0);
                } 
                else if (bytes_read == 0) {
                    // [정상 종료] 상대방이 close()로 TCP 연결을 끊은 경우
                    printf("[종료] 클라이언트가 연결을 닫았습니다 (fd: %d)\n", fds[i].fd);
                    close(fds[i].fd);
                    fds[i].fd = -1; // 슬롯 초기화하여 재사용 가능 상태로 변경
                } 
                else {
                    // [오류 또는 비동기 예외]
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        perror("recv 에러");
                        close(fds[i].fd);
                        fds[i].fd = -1;
                    }
                }
            }

            // 클라이언트 소켓에 예외/에러/강제종료(POLLHUP, POLLERR, POLLNVAL)가 발생한 경우
            if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                printf("[에러/끊김] 비정상 소켓 정리 (fd: %d)\n", fds[i].fd);
                close(fds[i].fd);
                fds[i].fd = -1;
            }
        }
    }

    // 서버 종료 처리
    close(server_fd);
    return 0;
}