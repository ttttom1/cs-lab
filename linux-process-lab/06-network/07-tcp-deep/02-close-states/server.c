#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    int opt = 1;
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
        exit(1);
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server listening...\n");

    int client_fd =
        accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        exit(1);
    }

    printf("client connected fd=%d\n", client_fd);

    char buffer[1024];

    while (1) {

        ssize_t n = recv(
            client_fd,
            buffer,
            sizeof(buffer) - 1,
            0
        );

        if (n > 0) {

            buffer[n] = '\0';

            printf("recv: %s\n", buffer);

        } else if (n == 0) {

            printf("recv returned 0\n");
            printf("FIN received from client\n");

            /*
             * 일부러 close하지 않는다.
             *
             * 여기서 server socket은
             * CLOSE_WAIT 상태를 관찰할 수 있다.
             */
            printf("sleeping 30 seconds before close...\n");

            sleep(30);

            printf("server closing socket\n");

            close(client_fd);

            break;

        } else {

            perror("recv");
            break;
        }
    }

    close(server_fd);

    return 0;
}