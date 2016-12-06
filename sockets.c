
// CS 356 - Sockets
// Maurice Achtenhagen (2016)
// Using Bellman–Ford algorithm

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define h_addr h_addr_list[0]
#define MAX_ROUTERS 4

void add();
void list();
void lct();
void displaytable();
void sendtable();
void updatetable();
void receive();
void bellmanford();
void error();

int id, mode, port;
typedef struct {
    int u, v, w;
} Edge;

const int NODES = 4; // number of nodes
int EDGES;           // number of edges
Edge edges[16];      // large enough for n <= 2^NODES = 16
int d[16];           // d[i] is the minimum distance from source node s to node i
char *routers[MAX_ROUTERS]; // list of current routers
char *initval; // Initial values for router

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage %s id port", argv[0]);
        exit(0);
    }
    id = atoi(argv[1]);
    port = atoi(argv[2]);
    mode = (id == 0 ? 1 : 0);
    lct();
    printf("CS 356 - Sockets\n");
    printf("Maurice Achtenhagen (2016)\n");
    printf("Using Bellman–Ford algorithm\n");
    printf("----------------------------\n");
    printf("Router ID = %d , port = %d\n", id, port);
    printf("Running in mode %d", mode);
    if (mode == 1) {
        printf(" (send)\n\n");
    } else {
        printf(" (listen)\n\n");
    }
    printf("The commands are as follows :\n\n");
    printf("(1) add \tAdd new router (4 max).\n");
    printf("(2) list \tList current routers.\n");
    printf("(3) send\tSend least cost table to other routers.\n");
    printf("(4) wait\tWait for incoming data.\n");
    printf("(5) table\tDisplay least cost table.\n");
    printf("(6) exit\tQuit the program.\n\n");    
    size_t bufsize = 32;
    char *buffer = (char *)malloc(bufsize * sizeof(char));       
    while (atoi(buffer) != 6) {       
        buffer = (char *)malloc(bufsize * sizeof(char));
        if (buffer == NULL) {
            perror("Unable to allocate buffer");
            exit(1);
        }
        printf("> ");
        getline(&buffer, &bufsize, stdin);        
        if (atoi(buffer) == 1) {
            add();
        } else if (atoi(buffer) == 2) {
            list();
        } else if (atoi(buffer) == 3) {
            if (mode == 0) {
                printf("Must be running in mode 1.\nMode will change upon receiving data\n");
            } else {
                sendtable();
            }
        } else if (atoi(buffer) == 4) {
            if (mode == 1) {
                printf("Must be running in mode 0.\nMode will change upon sending data\n");
            } else {
                receive();
            }
        } else if (atoi(buffer) == 5) {
            displaytable();
        } else {
            if (atoi(buffer) != 6) {
                printf("> Invalid command. Options: [1, 2, 3, 4, 5]\n");
            } else {
                printf("Bye!\n");
            }
        }
    }
    return 0;
}

// Add new router (4 max)
void add() {
    size_t bufsize = 32;
    char *buffer = (char *)malloc(bufsize * sizeof(char));
    printf("> Router IP address: ");
    getline(&buffer, &bufsize, stdin);
    for (int i = 0; i < 4; i++) {
        if (routers[i] == NULL) {
            routers[i] = buffer;
            printf("Added router with IP: %s", routers[i]);
            break;
        }
    }    
}

// List current routers
void list() {
    for (int i = 0; i < 4; i++) {
        printf("Router %d -> %s", i, routers[i]);
        if (routers[i] == NULL) {
            printf("\n");
        }
    }
}

// Send least cost table to another router
void sendtable() {
    int sockfd, max_conn, num_conn, rid;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    mode = 1;
    max_conn = num_conn = 0;
    rid = -1;    
    for (int i = 0; i < MAX_ROUTERS; i++) {
        if (i == id) { continue; }
        if (routers[i] == NULL) { continue; }
        max_conn++;
    }    
    do {
        sleep(1);
        rid++;        
        if (rid == 4) { rid = 0; }
        if (rid == id) { continue; }
        if (routers[rid] == NULL) { continue; }
        char *host = routers[rid];
        printf("Attempting to connect to %s", host);
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            error("Failed to open socket\n");
            continue;
        }
        server = gethostbyname(host);
        if (server == NULL) {
            error("No such host (bad hostname)\n");
            continue;
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(port);
        // connect() takes 3 args: file descriptor, address of host, size of address
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            error("Unable to connect to host\n");
            continue;
        }
        printf("Connection OK\n");   
        int n = write(sockfd, initval, strlen(initval));
        if (n < 0) {
            error("Failed to write to socket");
            continue;
        }
        close(sockfd);
        num_conn++;
    } while (num_conn < max_conn);
    printf("Mode has changed to 0\n");
    receive();
}

//  Receive
//  - Wait for clients to connect
//  - Receive least cost table from each other router
//  - Update least cost to each other router
//  - Display least cost table to each router

void receive() {
    int sockfd, newsockfd, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    mode = 0;    
    printf("Waiting for incoming data...\n");
    // socket() takes 3 args: address domain, socket type, protocol
    // Unix domain: AF_UNIX, Internet: AF_INET
    // TCP: SOCK_STREAM, UDP: SOCK_DGRAM
    // Returns entry into file descriptor table, -1 otherwise
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Failed to open socket");
    }
    // bzero() takes two args: pointer to buffer, size of buffer
    bzero((char *) &serv_addr, sizeof(serv_addr)); // Set all values to zero in buffer    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); // htons() converts a port number in host byte order to a port  
    serv_addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY gets the IP address of the machine
    // bind() takes 3 args: file descriptor, bind address, size of address
    // Binds the socket to an address
    bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    // listen() takes 2 args: file descriptor, backlog size (max waiting connections)
    // Allows the process to listen on the socket for connections
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    // Main loop
    while (1) {
        // accept() causes the process to block until a client connects to the server
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            error("Failed to accept connection");
        }
        pid = fork();
        if (pid < 0) {
            error("Failed to fork process");
        }
        if (pid == 0) {
            updatetable(newsockfd);
            close(newsockfd);
            mode = 1;
            printf("Mode has changed to 1\n");
            break;
        } else {
            close(newsockfd);
        }
    }
    close(sockfd);
}

// Construct least cost table
void lct() {
    int i, j, k, w;
    char *c = (char *)malloc(sizeof(char));
    initval = (char *)malloc(sizeof(char));
    k = 0;
    FILE *f = fopen("lct.txt", "r");
    for (i = 0; i < NODES; ++i) {
        for (j = 0; j < NODES; ++j) {
            fscanf(f, "%s", c);                        
            w = atoi(c);
            edges[k].u = i;
            edges[k].v = j;
            edges[k].w = w;
            k++;
            if (i == id) {                
                strcat(initval, c);
                strcat(initval, " ");
            }
        }
    }
    initval[strlen(initval)-1] = 0;
    fclose(f);
    EDGES = k;
}

// Display least cost table
void displaytable() {
    printf("\nShortest path from router %d to other routers:\n\n", id);
    for (int i = 0; i < NODES; ++i) printf("Node %d\t", i);
    printf("\n------------------------------\n");
    bellmanford(id);
    for (int i = 0; i < NODES; ++i) printf("%d\t", d[i]);
    printf("\n------------------------------\n\n");
}

// Update least cost table
void updatetable(int sock) {
    char buffer[256];
    bzero(buffer, 256);
    int n = read(sock, buffer, 255);
    if (n < 0) { error("Failed to read from socket"); }
    printf("Updating table with the following values: %s\n", buffer);
    char *token = strtok(buffer, " ");
    for (int i = 0; i < EDGES; ++i) {
        if (edges[i].u != 1) { continue; }
        if (token != NULL) {
            int num = atoi(token);
            if (num != 0) {
                edges[i].w = num;
            }
            token = strtok(NULL, " ");
        }
    }
    displaytable();
}

// Main algorithm
void bellmanford(int src) {
    int i, j;
    for (i = 0; i < NODES; ++i) { d[i] = INT_MAX; }
    d[src] = 0;
    for (i = 0; i < NODES - 1; ++i) {
        for (j = 0; j < EDGES; ++j) {
            if (d[edges[j].u] + edges[j].w < d[edges[j].v]) {
                d[edges[j].v] = d[edges[j].u] + edges[j].w;
            }
        }
    }
}

void error(char *msg) {
    perror(msg);
}
