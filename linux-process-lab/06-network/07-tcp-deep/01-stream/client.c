#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080

int main(void) {

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    if (inet_pton(
            AF_INET,
            "127.0.0.1",
            &addr.sin_addr
        ) != 1) {

        perror("inet_pton");
        close(fd);
        exit(1);
    }

    if (connect(
            fd,
            (struct sockaddr *)&addr,
            sizeof(addr)
        ) == -1) {

        perror("connect");
        close(fd);
        exit(1);
    }

    send(fd, "HELLO", 5, 0);
    send(fd, "WORLD", 5, 0);
    send(fd, "ABCDE", 5, 0);

    sleep(2);

    close(fd);

    return 0;
}