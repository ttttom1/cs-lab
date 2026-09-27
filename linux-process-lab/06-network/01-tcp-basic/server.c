#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

int main(void) {
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    printf("server_fd: %d\n", server_fd);

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

    struct sockaddr_in addr = { 0 };

    addr.sin_family = AF_INET;

    addr.sin_port = htons(PORT);

    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("listening of port %d...\n", PORT);

    sleep(30);

    close(server_fd);

    return 0;
}