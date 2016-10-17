// Sockets Example - Server
// A simple TCP server that listens for connections on a specified port

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void handlerequest(int);
void error (char *msg) {
    perror(msg);
    exit(1);
}

int main (int argc, char *argv[]) {
    int sockfd, newsockfd, port, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr, "Error, no port number specified.");
        exit(1);
    }
    printf("Waiting for client...\n");
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
    // Main loop
    while (1) {
        // accept() causes the process to block until a client connects to the server
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            error("Failed to accept connection.");
        }
        pid = fork();
        if (pid < 0) {
            error("Failed on fork.");
        }
        if (pid == 0) {
            close(sockfd);
            handlerequest(newsockfd);
            exit(0);
        } else {
            close(newsockfd);
        }
    }
    close(sockfd);
    return 0;
}

void handlerequest (int sock) {
    int n;
    char buffer[256];
    bzero(buffer, 256);
    n = read(sock, buffer, 255);
    if (n < 0) {
        error("Failed to read from socket.");
    }
    printf("Incoming message:\n%s\n", buffer);
    // Server sends its own Router number, and its initial known least cost to each other router
    char msg[] = "Router number: 1\nInitial known least cost to other routers:\n1 -> 0 = 1\n1 -> 2 = 1\n";
    n = write(sock, msg, sizeof(msg));
    if (n < 0) {
        error("Failed to write to socket.");
    }
}
