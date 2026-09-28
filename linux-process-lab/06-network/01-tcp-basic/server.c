#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUF_SIZE 1024
int main(void) {
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    //열려있다면, 무시하고 다시 덮어씀
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

    //IPv4 주소 구조체 생성, 설정
    struct sockaddr_in addr = { 0 };

    addr.sin_family = AF_INET;

    addr.sin_port = htons(PORT);

    addr.sin_addr.s_addr = INADDR_ANY;
    //설정한 주소 정보를 서버 소켓에 바인딩
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(1);
    }
    //소켓을 연결 수신 대기  상태로 전환
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("listening of port %d...\n", PORT);

    // 여기서 block
    int client_fd = accept(server_fd, NULL,NULL);

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        exit(1);
    }

    printf("client connected\n");
    printf("server_fd = %d\n", server_fd);
    printf("client_fd = %d\n", client_fd);

    char buffer[BUF_SIZE] = { 0 };

    ssize_t n = recv(
        client_fd,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (n == -1) {
        perror("recv");
        close(client_fd);
        close(server_fd);
        exit(1);
    }

    buffer[n] = '\0';

    printf("recieved: %s\n", buffer);

    const char *response = "hello client";

    if (send(
        client_fd,
        response,
        strlen(response),
        0
        ) == -1) {
        perror("send");
    }
    close(client_fd);
    close(server_fd);

    return 0;
}