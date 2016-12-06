# sockets-example
A simple example of a sockets program using the Bellman–Ford algorithm

This example demonstrates the ability for four routers to exchange data between themselves over the network. Every time a router changes the data in its table, it will send its entire table to all directly connected routers. Once the shortest paths have been computed, the program will terminate.

## Usage

Compile sockets.c using a C compiler like gcc. The sockets program requires 2 arguments, a router ID and port number. The client and server programs are now merged into one executable.

## Overview

### Client side

- Create the socket using the socket() system call.
- Connect to the server using the connect() system call.
- Send and receive data over the network.

### Server side

- Create and bind the socket to an address using socket() and bind() system calls.
- Listen for connections using the listen() system call.
- Accept connections using the accept() system call.
- Send and receive data over the network.

## Socket Types

Two processes can only communicate if they are using the same socket types. Furthermore, they must also be using the same address domain:
- Unix domain (processes share a common filesystem).
- Internet domain (processes communicate from anywhere).

Each of these types has its own address format. The address of unix domain socket is made up of a character string, which is essentially an entry in the filesystem. The address of an internet socket in the internet domain contains a unique 32-bit address (IP address). Additionally, each type also requires a port number its host. Port numbers are unsigned 16-bit integers. Lower port numbers are reserved in Unix. Generally, ports above 2000 are available to use.

### Stream Sockets

- Uses TCP (Transmission Control Protocol).
- Reliable and stream oriented.

### Datagram Sockets

- Uses UDP (User Datagram Protocol).
- Unreliable and message oriented.

## Sending a message from stdin
```C
printf("Please enter the message: ");
bzero(buffer, 256);
fgets(buffer, 255, stdin);
n = write(sockfd, buffer, strlen(buffer));
if (n < 0) {
    error("Failed to write to socket.");
}
```


### References: http://www.linuxhowtos.org/C_C++/socket.htm
