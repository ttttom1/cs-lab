#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h> // 소켓 생성 및 통신 관련 함수 (socket, connect, send, recv 등)
#include <arpa/inet.h>   // IP 주소 변환 및 바이트 순서 변환 함수 (htons, inet_pton 등)

#define PORT 8080
#define BUF_SIZE 1024

int main(void) {
    // 1. TCP 소켓 생성 (IPv4, TCP 통신)
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        perror("socket");
        exit(1);
    }

    // 2. 서버 주소 정보 구조체 설정 (IPv4, Port 8080)
    struct sockaddr_in server_addr = { 0 }; // 쓰레기 값 방지를 위한 초기화
    server_addr.sin_family = AF_INET;       // IPv4 주소 체계
    server_addr.sin_port = htons(PORT);     // Port 번호를 네트워크 바이트 순서(Big-Endian)로 변환

    // 문자열 IP 주소("127.0.0.1")를 바이너리 네트워크 주소로 변환하여 server_addr.sin_addr에 저장
    if (inet_pton(
        AF_INET,
        "127.0.0.1",
        &server_addr.sin_addr
    ) != 1){

        perror("inet_pton");
        close(sock_fd);
        exit(1);
    }
    
    // 3. 서버에 TCP 연결 요청 (3-way handshake 수행)
    if (connect(
        sock_fd,
        (struct sockaddr *)&server_addr,
        sizeof(server_addr)
    ) == -1) {

        perror("connect");
        close(sock_fd);
        exit(1);
    }

    printf("connected to server\n");

    const char *message  = "hello server";

    // 4. 서버로 메시지 전송
    if (send(
        sock_fd,
        message,
        strlen(message),
        0
    ) == -1) {
        perror("send");
        close(sock_fd);
        exit(1);
    }

    char buffer[BUF_SIZE] = {0};

    // 5. 서버로부터 응답 수신 (데이터가 올 때까지 대기/Blocking)
    ssize_t n = recv(
        sock_fd,
        buffer,
        sizeof(buffer) - 1, // 문자열 끝 null 문자를 남겨두기 위해 -1
        0
    );

    if (n  == -1) {
        perror("recv");
        close(sock_fd);
        exit(1);
    }
    // 수신한 데이터 끝에 null 문자('\0')를 붙여 안전한 문자열로 완결
    buffer[n] = '\0';

    printf("server response: %s\n", buffer);

    // 6. 소켓 자원 해제 및 연결 종료
    close(sock_fd);

    return 0;
}