#include "scheduler.hpp"


int main() {    
    Scheduler scheduler;
    // 1. UDP: create server
    int udp_fd = scheduler.create_bind_UDP();
    if (udp_fd == -1) {
        return EXIT_FAILURE;
    }
    std::cout << "\nThe Scheduler is up and running." << std::endl;
    // 2. UDP: recv initialized data from hospitals
    std::map<int, std::string> init_response = scheduler.recvUDPfromHosp(3, udp_fd);
    // store information to the specific hospital

    for (auto& n: init_response) {
        std::vector<std::string> res = scheduler.splitMessage(n.second, ' ');
        int cur_port = n.first;
        if (cur_port == PORTA) {
            scheduler.hospital_info["A"] = std::make_tuple(res[1], std::stoi(res[2]), std::stoi(res[3]));
            std::cout << "The Scheduler has received information from Hospital A: total capacity is " << res[2] << " and initial occupancy is " << res[3] << std::endl;
        }
        else if (cur_port == PORTB) {
            scheduler.hospital_info["B"] = std::make_tuple(res[1], std::stoi(res[2]), std::stoi(res[3]));
            std::cout << "The Scheduler has received information from Hospital B: total capacity is " << res[2] << " and initial occupancy is " << res[3] << std::endl;
        }
        else if (cur_port == PORTC) {
            scheduler.hospital_info["C"] = std::make_tuple(res[1], std::stoi(res[2]), std::stoi(res[3]));
            std::cout << "The Scheduler has received information from Hospital C: total capacity is " << res[2] << " and initial occupancy is " << res[3] << std::endl;
        }
    }
    // 3. TCP: create server
    int tcp_fd = scheduler.create_bind_TCP();
    if (tcp_fd == -1) {
        return EXIT_FAILURE;
    }
    int listen_msg = listen(tcp_fd, BACKLOG);
    if (listen_msg == -1) {
        perror("TCP: listen");
        return EXIT_FAILURE;
    }
    std::cout << "\n----------------------------\n" << std::endl;
    // loop in 4 - 6 
    while(1) {
        // 4. TCP: recv data from clients
        std::pair<int, std::string> cfd_loc = scheduler.recvTCPdata(tcp_fd);
        if (cfd_loc.second == "") {
            perror("Client port is empty");
            continue;
        }
        std::cout << "The Scheduler has received client at location " << cfd_loc.second << " from the client TCP over port " << PORT_CLIENT << std::endl;

        // 5. UDP: send data to hospital
        std::set<std::string> avail_hosp = scheduler.availHospitals();
        for (auto& p: avail_hosp) {
            scheduler.sendUDPdata(udp_fd, cfd_loc.second, name_port[p]);
            std::cout << "The Scheduler has sent location to Hospital " << p <<" using UDP over port " << PORT_HOSPITAL << std::endl;
        }
        if (avail_hosp.empty()) {
            std::cout << "All hospitals are in maximum occupancy" << std::endl;
            std::string msg = "FULL";
            scheduler.sendTCPtoClient(cfd_loc.first, msg);
            std::cout << "The Scheduler has sent the result to client using TCP over port " << PORT_CLIENT << '\n' << std::endl;
            continue;
        }
        std::cout << std::endl;

        // recv data from available hospital
        // response: {port number: (port score dist)}
        std::map<int, std::string> response = scheduler.recvUDPfromHosp(avail_hosp.size(), udp_fd);

        // return code: A / B / C / NOTFOUND / Empty / NOAssignment
        std::string best = scheduler.cmpResponse(response);
        std::cout << std::endl;

        // 6. UDP / TCP : send decision to hospitals / client
        int child_cfd = cfd_loc.first;
        if (best == "None" || best == "NOTFOUND" || best == "NOAssignment") {
            std::cout << "The Scheduler has assigned Hospital None to the client" << std::endl;
            scheduler.sendTCPtoClient(child_cfd, best);
            std::cout << "The Scheduler has sent the result to client using TCP over port " << PORT_CLIENT << std::endl;

        } 
        else if (best == "Empty") {
            std::cout << "The Scheduler has assigned Hospital None to the client" << std::endl;
        }
        else {
            std::cout << "The Scheduler has assigned Hospital " << best << " to the client" << std::endl;
            scheduler.sendTCPtoClient(child_cfd, best);
            std::cout << "The Scheduler has sent the result to client using TCP over port " << PORT_CLIENT << std::endl;
        }

        close(child_cfd);

        for (auto &n: name_port) {
            if (n.first == best) {
                scheduler.sendUDPdata(udp_fd, "Chosen", n.second);
                std::cout << "The Scheduler has sent the result to Hospital " << n.first << " using UDP over port " << PORT_HOSPITAL << std::endl;
                scheduler.updateHospInfo(best);
            }
            else {
                scheduler.sendUDPdata(udp_fd, "Not Chosen", n.second);
            }
        }        
        std::cout << "\n----------------------------\n" << std::endl;
        
    }
    close(tcp_fd);
    close(udp_fd);
    return EXIT_SUCCESS;

}