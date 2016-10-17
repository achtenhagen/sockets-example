// Sockets Example - Client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(char *msg) {
    perror(msg);
    exit(0);
}

int main (int argc, char *argv[]) {
    int sockfd, port, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    printf("Client\n");
    char buffer[256];
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port", argv[0]);
        exit(0);
    }
    port = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Failed to open socket.");
    }
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Error - no such host.");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    // connect() takes 3 args: file descriptor, address of host, size of address
    // Establishes connection with server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("Failed to connect.");
    }
    // Client displays its initial known least cost to each other router
    // Client sends its own Router number, and its initial known least cost to each other router
    char msg[] = "Router number: 0\nInitial known least cost to other routers:\n0 -> 1 = 1\n0 -> 2 = 3\n0 -> 3 = 7\n";
    printf("%s\n", msg);
    n = write(sockfd, msg, strlen(msg));
    if (n < 0) {
        error("Failed to write to socket.");
    }
    // Client displays information received from server
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) {
        error("Failed to read from socket.");
    }
    printf("Response from server:\n%s", buffer);

    return 0;
}
