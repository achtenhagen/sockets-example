// Sockets example - Server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error (char *msg) {
    perror(msg);
    exit(1);
}

int main (int argc, char *argv[]) {
    int sockfd, newsockfd, port, clilen, n;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr, "Error, no port number specified.");
        exit(1);
    }
    // socket() takes 3 args: address domain, socket type, protocol
    // Unix domain: AF_UNIX, Internet: AF_INET
    // TCP: SOCK_STREAM, UDP: SOCK_DGRAM
    // Returns entry into file descriptor table, -1 otherwise
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error opening socket.");
    }
    // bzero() takes two args: pointer to buffer, size of buffer
    // Set all values to zero in buffer
    bzero((char *) &serv_addr, sizeof(serv_addr));
    // atoi() converts a string into an integer
    port = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    // htons() converts a port number in host byte order to a port
    // number in network byte order
    serv_addr.sin_port = htons(port);
    // INADDR_ANY gets the IP address of the machine
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // bind() takes 3 args: file descriptor, bind address, size of address
    // Binds the socket to an address
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("Failed to bind socket.");
    }
    // listen() takes 2 args: file descriptor, backlog size (max waiting connections)
    // Allows the process to listen on the socket for connections
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    // accept() causes the process to block until a client connects to the server
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        error("Failed to accept connection.");
    }
    bzero(buffer, 256);
    n = read(newsockfd, buffer, 255);
    if (n < 0) {
        error("Failed to read from socket.");
    }
    printf("Here is the message: %s\n", buffer);
    n = write(newsockfd, "I got your message", 18);
    if (n < 0) {
        error("Failed to write to socket.");
    }

    return 0;
}
