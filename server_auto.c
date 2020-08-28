// socket server example, handles multiple clients using threads

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread

// The thread function
void *connection_handler(void *);

int main(int argc, char *argv[]) {
    // Check program's arguments
    if (argc < 2) {
        printf("./server port\n");
        return 1;
    }

    // Vars
    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server, client;
    char ip[100];
    int port;

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
        printf("Could not create socket");

    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));

    // Timeout
    struct timeval tv;
    tv.tv_sec = 60;
    tv.tv_usec = 0;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof tv) < 0) {
        printf("Could not set SO_RCVTIMEO socket");
        return -1;
    }
    if (setsockopt(socket_desc, SOL_SOCKET, SO_SNDTIMEO, (const char *) &tv, sizeof tv) < 0) {
        printf("Could not set SO_SNDTIMEO socket");
        return -1;
    }

    // Bind
    if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("Bind failed");
        return 1;
    }
    puts("Bind done");

    // Listen
    listen(socket_desc, 3);

    // Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    while ((client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t * ) & c))) {
        if (client_sock == -1) {
            perror("Accept failed");
            continue;
        }

        inet_ntop(AF_INET, &(client.sin_addr), ip, INET_ADDRSTRLEN);
        port = (int) ntohs(client.sin_port);

        printf("Connection accepted: %s:%d %d\n", ip, port, client_sock);

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *) new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }

        puts("Handler assigned");
    }

    if (client_sock < 0) {
        perror("Accept failed (end)");
        return 1;
    }

    return 0;
}

/**
 * @param char* pre
 * @param char* str
 * @return _Bool Строка str начинается на pre
 */
_Bool startsWith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre),
            lenstr = strlen(str);
    return lenstr < lenpre ? 0 : memcmp(pre, str, lenpre) == 0;
}

/**
 * This will handle connection for each client
 * @param socket_desc
 * @return
 */
void *connection_handler(void *socket_desc) {
    // Get the socket descriptor
    int sock = *(int *) socket_desc;
    int n;
    _Bool isHttp = 0;
    char message[2000] = {0};
    char client_message[2000], resultMessage[2000];

    while (memset(client_message, 0, sizeof(client_message))
           && (n = recv(sock, client_message, 2000, 0)) > 0) {
        printf("> %s", client_message);

        if (!isHttp && startsWith("GET ", client_message)) {
            isHttp = 1;
            printf("isHttp = 1\n");
        }

        if (isHttp) {
            strcat(message, client_message);
            printf("message = %s", message);

            if (strstr(message, "\r\n\r\n") != NULL) {
                printf("Message complete\n");

                sprintf(resultMessage, "HTTP/1.1 200 OK\r\nContent-type: text/plain; charset=UTF-8\r\n\r\nOK\r\n");
                send(sock, resultMessage, strlen(resultMessage), 0);
                // strcpy(message, {0});
                memset(message, 0, sizeof(message));
            }

            continue;
        }


        send(sock, client_message, n, 0);
        printf("< %s", client_message);
    }
    close(sock);

    printf("Recv bytes: %d\n", n);

    if (n == 0)
        puts("Client Disconnected");
    else
        perror("recv failed");

    return 0;
}
