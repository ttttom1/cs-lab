#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024

int main(void) {

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in addr = { 0 };

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(fd);
        exit(1);
    }

    send(fd, "Hello", 5, 0);
    send(fd, "World", 5, 0);

    printf("finished sending\n");

    // half-close
    shutdown(fd, SHUT_WR);

    printf("write side closed\n");

    char buffer[BUF_SIZE] = { 0 };

    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);

    if (bytes_read == -1) {
        perror("read");
        close(fd);
        exit(1);
    }

    buffer[bytes_read] = '\0';
    printf("received: %s\n", buffer);

    close(fd);
    return 0;
}