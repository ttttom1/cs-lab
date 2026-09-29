#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUF_SIZE 1024

// 클라이언트 연결을 전담 처리할 워커 스레드 함수
void* handle_client(void* arg) {

    // [TODO 1] void* 인자로 넘겨받은 동적 할당 포인터를 int*로 형변환 후 값 꺼내기
    int client_fd = *(int*)arg;

    // [TODO 2] main 스레드가 malloc()으로 할당해준 메모리 해제 (메모리 누수 방지)
    free(arg);

    char buffer[BUF_SIZE] = {0};

    printf(
        "[worker] fd=%d waiting for message...\n",
        client_fd
    );

    // [TODO 3] 클라이언트로부터 데이터 수신 (I/O 블로킹 발생 지점 - 해당 스레드만 블로킹됨)
    ssize_t n = recv(
        client_fd,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (n <= 0) {
        if (n == -1) {
            perror("[worker] recv error");
        } else {
            printf("[worker] client fd=%d disconnected\n", client_fd);
        }
        // 수신 실패 또는 연결 종료 시 소켓 닫고 스레드 종료
        close(client_fd);
        return NULL;
    }

    // [TODO 4] 수신한 문자열 끝에 NULL 문자 채우고 내용 출력
    buffer[n] = '\0';
    printf(
        "[worker] received from fd=%d: %s\n",
        client_fd,
        buffer
    );

    // [TODO 5] 클라이언트에 응답 전송
    const char *response = "server received message";
    send(
        client_fd,
        response,
        strlen(response),
        0
    );

    // [TODO 6] 해당 클라이언트와의 통신이 끝났으므로 소켓 닫기
    close(client_fd);
    printf("[worker] client fd=%d closed\n", client_fd);

    return NULL;
}

int main(void) {

    // 1. 소켓 생성 (IPv4, TCP)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    // 2. 포트 재사용 옵션 설정 (SO_REUSEADDR)
    int opt = 1;
    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    // 3. 주소 구조체 설정 및 바인딩
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

    // 4. 리스닝 상태 전환 (연결 대기열 생성)
    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("server listening on port %d...\n", PORT);

    while (1) {

        // Main 스레드는 계속해서 새로운 연결만 수락(accept)함
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        printf(
            "[main] client connected fd=%d\n",
            client_fd
        );

        // [TODO 7] client_fd 저장용 malloc
        // 단순 &client_fd의 주소를 넘기면 다음 loop에서 accept()할 때 값이 덮어씌워지는 Race Condition이 발생하므로
        // 힙(Heap) 공간에 독립적인 메모리를 할당해 넘겨주어야 함
        int *arg = malloc(sizeof(int));
        if (arg == NULL) {
            perror("malloc");
            close(client_fd);
            continue;
        }
        *arg = client_fd;

        pthread_t thread;

        // [TODO 8] 워커 스레드 생성 (handle_client 실행, malloc한 arg 전달)
        if (pthread_create(&thread, NULL, handle_client, arg) != 0) {
            perror("pthread_create");
            free(arg);
            close(client_fd);
            continue;
        }

        // [TODO 9] 스레드 분리 (pthread_detach)
        // Main 스레드가 pthread_join으로 기다리지 않고, 워커 스레드가 종료될 때 OS가 자원을 자동으로 회수하도록 함
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}