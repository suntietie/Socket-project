// #include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

// C++ standard library
#include <string>
#include <iostream>

#define PORT_CLIENT 34942
#define MAX_MSG_SIZE 1024
#define LOCALHOST "127.0.0.1"
// TCP client

int main(int argc, char* argv[]) {
    char* loc_client = argv[1];
    if (int(*loc_client) < 0) {
        std::cout << "Please input a valid location" << std::endl;
        return EXIT_FAILURE;
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT_CLIENT);
    inet_pton(AF_INET, LOCALHOST, &serveraddr.sin_addr.s_addr);
    
    int sock_client = connect(sockfd, (struct sockaddr*)& serveraddr, sizeof(serveraddr));

    if (sock_client == -1) {
        close(sockfd);
        perror("client: connect");
        return EXIT_FAILURE;
    }
    char *sendbuf = loc_client;
    int bytes_sent, byte_recv;
    std::cout << "\nThe client is up and running" << std::endl;
    // 1.TCP: send data to scheduler
    bytes_sent = send(sockfd, sendbuf, strlen(sendbuf)+1, 0);
    if (bytes_sent == -1) {
        perror("TCP send");
        return EXIT_FAILURE;
    }
    std::string send_str = loc_client;
    std::cout << "The client has sent query to Scheduler using TCP: client location ​" << send_str << std::endl;
    // 2. TCP: recv data from scheduler
    char recvbuf[MAX_MSG_SIZE];
    
    byte_recv = recv(sockfd, recvbuf, MAX_MSG_SIZE, 0);
    if (byte_recv == -1) {
        perror("TCP recv");
        return EXIT_FAILURE;
    }
    std::string recvstring = recvbuf;
    // return code: A / B / C / NOTFOUND  / NOAssignment / FULL

    if (recvstring == "FULL") {
        std::cout << "The client has received results from the Scheduler: assigned to Hospital None" << std::endl;
    }
    else if (recvstring == "NOTFOUND") {
        std::cout << "Location " << send_str << " not found" << std::endl;
    }
    else if (recvstring == "NOAssignment") {
        std::cout << "Score = None, No assignment" << std::endl;
    }
    else if (recvstring == "A" || recvstring == "B" || recvstring == "C") {
        std::cout << "The client has received results from the Scheduler: assigned to Hospital " << recvstring << std::endl;
    }
    // 预留
    else {
        std::cout << "The client has received results from the Scheduler: assigned to Hospital None" << std::endl;
    }

    std::cout << std::endl;
    close(sockfd);
    return EXIT_SUCCESS;

}