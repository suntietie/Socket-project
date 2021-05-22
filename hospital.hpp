#ifndef HOSPITAL_HPP
#define HOSPITAL_HPP

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
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <queue>

//TODO: change the input file here
#define FILE "map_hard.txt"
#define LOCALHOST "127.0.0.1"
#define PORT_HOSPITAL 33942
#define MAXBUFLEN 1024

class Hospital {

public:

    // function declaration

    void fileParser();
    void printGraph();

    double shortestDistance(std::string loc_a, std::string loc_b);
    double calScore(double dist);
    double calAvail();

    int setParam(std::string loc, int capa, int occu, int p);
    int checkValid(std::string loc);


    int create_bind_UDP();

    void sendUDPdata(int fd, std::string msg, int dest_port);

    std::pair<int, std::string> recvUDPData(int fd);


    // class attribute

    const char* file = FILE;
    // data structure to store graph in adjacency list
    std::unordered_map<std::string, std::unordered_map<std::string, double> > map_graph;
    std::string location;
    int capacity;
    int occupancy;
    int port;

};


/*
* Parse the map file into a graph
* Graph is illustrated by ajacency list.
*/

void Hospital::fileParser() {
    std::fstream map_in;
    map_in.open(file, std::ios::in);

    std::string line, word, node1, node2;
    double len;
    
    while (std::getline(map_in, line)) {
        std::stringstream ssline(line);
        // read stream, split by space
        int count = 0;
        while (std::getline(ssline, word, ' ')) {

            std::remove(word.begin(), word.end(), ' ');
            if (word == "") {
                continue;
            }
            if (count == 0) {
                node1 = word;
            }
            else if (count == 1){
                node2 = word;
            }
            // count = 2
            else if (count == 2){
                len = std::stod(word);
            }
            count ++;
        }
        if (node1 == node2) {
            continue;
        }
        // insert into the graph
        map_graph[node1][node2] = len;
        map_graph[node2][node1] = len;
    }

    return;
}

/*
* Find Shortest Path Algorithm - Dijkstra
* 
*/
double Hospital::shortestDistance(std::string loc_a, std::string loc_b) {

    double shortest_len;
    std::unordered_map<std::string, double> short_dist;
    std::unordered_set<std::string> visited;

    //initialize with max size
    for(auto &n: map_graph) {
        short_dist[n.first] = __DBL_MAX__;
    }

    short_dist[loc_a] = 0.0;
    auto my_cmp = [&short_dist](std::string& a, std::string& b) {
        return short_dist[a] > short_dist[b];
    };
    // construct a minimum heap
    std::priority_queue<std::string, std::vector<std::string>, decltype(my_cmp) > pq(my_cmp);
    pq.push(loc_a);

    while (!pq.empty()) {
        std::string top_loc = pq.top();
        pq.pop();
        visited.insert(top_loc);

        // find destination
        if (top_loc == loc_b) {
            shortest_len = short_dist[top_loc];
            break;
        }
        // expand its neighbor
        for(auto &neigh_pair : map_graph[top_loc]) {

            std::string neigh = neigh_pair.first;
            double neigh_len = neigh_pair.second;
            if (visited.find(neigh) != visited.end()) {
                continue;
            }
            // compare src->v with src->u + u->v
            if(short_dist[neigh] > short_dist[top_loc] + neigh_len) {
                // update distance
                short_dist[neigh] = short_dist[top_loc] + neigh_len;
                pq.push(neigh);
            }
        }
    }
    return shortest_len;
}


/*
* Calculate score:
* dist = shortest distance
* avail = (capacity - occupation) / capacity
* score = 1 / (d*(1.1-avail))
*/

double Hospital::calScore(double dist) {

    double avail = double(capacity - occupancy) / double(capacity);
    double score = 1.0 / (dist * (1.1 - avail));
    return score;

}

double Hospital::calAvail() {
    return double(capacity - occupancy) / double(capacity);
}
/*
* Check if the input parameters are legal or not
*/

int Hospital::setParam(std::string loc, int capa, int occu, int p) {

    if (map_graph.find(loc) == map_graph.end() || capa < 0 || occu < 0 || capa < occu) {
    std::cout << "Error: Hospital's Input parameters are illegal." << std::endl;
    return EXIT_FAILURE;
    }

    location = loc;
    capacity = capa;
    occupancy = occu;
    port = p;
    return EXIT_SUCCESS;

}

/*
* Print the graph in adjacancy list 
*/

void Hospital::printGraph() {

    std::cout.precision(17);
    for (auto &n: map_graph) {
        for (auto &p: map_graph[n.first]) {
            std::cout << n.first << ' ' << p.first << ' ' << p.second << std::endl;
        }
    }
    return;
}
/*
* Create UDP socket and bind static PORT number
*/
int Hospital::create_bind_UDP() {
    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd == -1) {
        perror("UDP socket");
        return -1;

    }
    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
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
* Send UDP data to the scheduler
*/
void Hospital::sendUDPdata(int fd, std::string msg, int dest_port) {
    int numByte;
    char buf[MAXBUFLEN];
    strcpy(buf, msg.c_str());

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dest_port);
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
std::pair<int, std::string> Hospital::recvUDPData(int fd) {

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
* Check valid location
* return 1 if error, else return 0
*/
int Hospital::checkValid(std::string loc) {

    if (map_graph.find(loc) == map_graph.end()) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

#endif