#pragma once

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

// #include "scheduler.h"
// C++ standard library
#include <vector>
#include <string>
#include <tuple>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#define PORTA 30942
#define PORTB 31942
#define PORTC 32942
#define PORT_HOSPITAL 33942
#define PORT_CLIENT 34942
#define LOCALHOST "127.0.0.1"
#define MAXBUFLEN 1024
#define BACKLOG 10
#define MINSCORE -1
#define MAXDIST __DBL_MAX__  
#define NOTFOUND -2
#define None -3


std::map<std::string, int> name_port = {{"A", PORTA}, {"B", PORTB}, {"C", PORTC}};
std::map<int, std::string> port_name = {{PORTA, "A"}, {PORTB, "B"}, {PORTC, "C"}};


class Scheduler {
public:
    // function declaration

    // UDP message    
    int create_bind_UDP();
    void sendUDPdata(int fd, std::string msg, int port);
    std::pair<int, std::string> recvUDPData(int fd);
    std::map<int, std::string> recvUDPfromHosp(const int hosp_num, int udp_fd);


    // TCP message
    int create_bind_TCP();
    void sendTCPtoClient(int cfd, std::string& msg);
    std::pair<int, std::string> recvTCPdata(int fd);

    

    std::vector<std::string> splitMessage(std::string& msg, const char split);

    std::string cmpResponse(std::map<int, std::string>& response);

    std::set<std::string> availHospitals();

    void updateHospInfo(std::string best);


    // class attribute

    // data structure to store hospital information
    // {hospitalA : {loc, capacity, occupancy}, ... }
    std::map<std::string, std::tuple<std::string, int, int> > hospital_info;

};

/*
* Create UDP socket and bind static PORT number
*/
int Scheduler::create_bind_UDP() {
    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd == -1) {
        perror("UDP socket");
        return -1;
    }
    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(33942);
    inet_pton(AF_INET, LOCALHOST, &local_addr.sin_addr.s_addr);
    // bind fd with address
    int udp_bind = bind(udp_fd, (const sockaddr* )&local_addr, sizeof(local_addr));
    if (udp_bind == 1) {
        perror("UDP Listener: bind");
        return -1;
    }
    return udp_fd;
}

/*
* Send UDP data to the hospital
*/
void Scheduler::sendUDPdata(int fd, std::string msg, int port) {
    int numByte;
    char buf[MAXBUFLEN];
    strcpy(buf, msg.c_str());

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    inet_pton(AF_INET, LOCALHOST, &dest_addr.sin_addr.s_addr);
    numByte = sendto(fd, &buf, strlen(buf)+1, 0, (struct sockaddr* ) &dest_addr, sizeof dest_addr);
    if (numByte == -1) {
        perror("UDP send error");
        exit(EXIT_FAILURE);
    }
    return;
}

/*
* Recv UDP data from the hospital
*/
std::pair<int, std::string> Scheduler::recvUDPData(int fd) {

    int numByte;
    char buf[MAXBUFLEN];
    struct sockaddr_in their_addr;
    socklen_t addr_len = sizeof(their_addr);
    numByte = recvfrom(fd, buf, MAXBUFLEN-1, 0, (struct sockaddr* )& their_addr, &addr_len);
    
    if (numByte == -1) {
        perror("recvfrom");
        return std::make_pair(0, "");
    }
    std::string res = buf;
    int port = ntohs(their_addr.sin_port);
    return std::make_pair(port, res);

}


/*
* Recv TCP data from the client
*/
std::pair<int, std::string> Scheduler::recvTCPdata(int fd) {
    std::string result;
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    int child_fd = accept(fd, (sockaddr* )& clientaddr, & clientaddr_len);

    if (child_fd == -1) {
        perror("TCP: accpet");
        return std::make_pair(child_fd, result);
    }
    int clientPort = ntohs(clientaddr.sin_port);

    char buf[MAXBUFLEN];
    int buf_size = recv(child_fd, &buf, MAXBUFLEN-1, 0);
    if (buf_size == -1) {
        perror("read");
    }
    else if (buf_size > 0) {
        result = buf;
    }

    return std::make_pair(child_fd, result);
}


/*
* Create TCP socket and bind with static port number
*/    
int Scheduler::create_bind_TCP() {
    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd == -1) {
        perror("TCP socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_CLIENT);
    inet_pton(AF_INET, LOCALHOST, &server_addr.sin_addr.s_addr);
    int tcp_bind = bind(tcp_fd, (const sockaddr* )&server_addr, sizeof(server_addr));
    if (tcp_bind == -1) {
        perror("TCP Listener bind");
        return -1;
    }
    return tcp_fd;
}

/*
* Recv all data from the hospitals
*/  
std::map<int, std::string> Scheduler::recvUDPfromHosp(const int hosp_num, int udp_fd) {
    
    std::map<int, std::string> response;
    std::set<int> hospitals;
    while (hospitals.size() < hosp_num) {
        std::pair<int, std::string> msg = recvUDPData(udp_fd);
        
        if (msg.first == 0) {
            return response;
        }
        int port = msg.first;
        std::string result = msg.second;
        hospitals.insert(port);
        response[port] = result;
    }
    return response;
}

/*
* TCP: send data to client
*/  
void Scheduler::sendTCPtoClient(int cfd, std::string& msg) {
    int numByte;
    char buf[MAXBUFLEN];
    strcpy(buf, msg.c_str());
    numByte = send(cfd, buf, strlen(buf)+1, 0);
    if (numByte == -1) {
        perror("TCP send error");
        exit(EXIT_FAILURE);
    }
    return;        
}

/*
* separate data with "split" char
* "This is a test" => {"This", "is", "a", "test"}
*/ 
std::vector<std::string> Scheduler::splitMessage(std::string& msg, const char split) {
    std::vector<std::string> res;
    std::string token;
    std::stringstream ssline(msg);
    while(getline(ssline, token, split)) {
        std::remove(token.begin(), token.end(), ' ');
        if (token == "") {
            continue;
        }
        res.push_back(token);
    }
    return res;
}

/*
* Update information in the hospital_info
* return type: A / B / C / NOTFOUND / Empty / NOAssignment
*/ 
std::string Scheduler::cmpResponse(std::map<int, std::string>& response) {

    if (response.empty()) {
        std::cout << "Response is empty" << std::endl;
        return "Empty";

    }

    double best_score = MINSCORE;
    double best_distance = MAXDIST;
    std::string best;
    std::vector<std::string> parsed;
    int flag = 0;
    for (auto &n: response) {
        parsed = splitMessage(n.second, ' ');
        // distance is illegal
        if (parsed[1] == "NOTFOUND" || parsed[2] == "NOTFOUND") {
            std::cout << "The Scheduler has received map information from Hospital " << port_name[n.first] << ", the score = None and the distance = None" << std::endl;
            flag = 1;
            continue;
        }   
        else if (parsed[2] == "None") {
            std::cout << "The Scheduler has received map information from Hospital " << port_name[n.first] << ", the score = None and the distance = None" << std::endl;
            flag = 2;
            continue;                
        }
        // distance is legal         
        else if (parsed[1] == "None") {
            std::cout << "The Scheduler has received map information from Hospital " << port_name[n.first] << ", the score = None and the distance = " << parsed[2] << std::endl;                
            continue;
        }
        double curr_score = std::stod(parsed[1]);
        double curr_distance = std::stod(parsed[2]);
        std::cout << "The Scheduler has received map information from Hospital " << port_name[n.first] << ", the score = " << curr_score << " and the distance = " << curr_distance << std::endl;
        if (best_score < curr_score) {
            best_score = curr_score;
            best_distance = curr_distance;
            best = port_name[n.first];
        }
        // if tie, find shortest distance
        else if (best_score == curr_score) {
            if (best_distance > curr_distance) {
                best_distance = curr_distance;
                best = port_name[n.first];
            }
        }
    }
    if (flag == 1) {
        return "NOTFOUND";
    }
    else if (flag == 2) {
        return "NOAssignment";
    }
    return best;
}


/*
* Find available hospitals 
*/ 
std::set<std::string> Scheduler::availHospitals() {
    std::set<std::string> hosp_set;
    for (auto &n: hospital_info) {
        if (std::get<1>(n.second) == std::get<2>(n.second)) {
            continue;
        }
        hosp_set.insert(n.first);
    }
    return hosp_set;
}

/*
* Update information in the hospital_info
*/ 

void Scheduler::updateHospInfo(std::string best) {
    std::tuple<std::string, int, int>& tup = hospital_info[best];
    std::get<2> (tup) ++;
    return;
}

