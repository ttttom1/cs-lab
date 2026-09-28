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

    int opt = 1;
    // 1. setsocket -> setsockopt 오타 수정
    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

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

    printf("server listening on port %d\n", PORT);

    while(1) {
        printf("waiting for client...\n");

        // 2. serveer_fd -> server_fd 오타 수정
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        printf("client connected. fd=%d\n", client_fd);

        char buffer[BUF_SIZE] = { 0 };

        printf("waiting for message from fd=%d...\n", client_fd);

        // 3. buffeer -> buffer 오타 수정
        ssize_t n = recv(
            client_fd,
            buffer,
            sizeof(buffer) - 1,
            0
        );

        if (n <= 0) {
            if (n == -1) {
                perror("recv");
            }

            close(client_fd);
            continue;
        }

        buffer[n] = '\0';

        printf(
            "recieved from fd=%d: %s\n",
            client_fd,
            buffer
        );

        const char *response = "server recieved message";

        send(
            client_fd,
            response,
            strlen(response),
            0
        );

        close(client_fd);

        printf("client fd=%d closed\n", client_fd);
    }

    close(server_fd);

    return 0;
}