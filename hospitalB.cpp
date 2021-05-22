// Utility Function
#include "hospital.hpp"


#define HOSPITAL_LOCAL_PORT 31942
#define HOSPITAL_NAME "B"
#define HOSPITAL_LOCAL_PORT_STR "31942"

int main(int argc, char* argv[]) {

    std::string location = argv[1];
    int capacity = std::stoi(argv[2]);
    int occupancy = std::stoi(argv[3]);

    std::string hosp_port = HOSPITAL_LOCAL_PORT_STR;
    std::string hosp_name = HOSPITAL_NAME;
    int port = HOSPITAL_LOCAL_PORT;

    // 1. Data parser
    Hospital hospitalB;
    hospitalB.fileParser();
    int fflag = hospitalB.setParam(location, capacity, occupancy, port);
    if (fflag == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    
    // 2. UDP: send capacity and initial occupancy to scheduler
    int fd = hospitalB.create_bind_UDP();
    std::string init_msg;
    init_msg = hosp_port + " " + location + " " + std::to_string(capacity) + " " + std::to_string(occupancy);
    hospitalB.sendUDPdata(fd, init_msg, PORT_HOSPITAL);
    // loop in 3 - 6
    std::cout << "\nHospital " << hosp_name << " is up and running using UDP on port " << hosp_port << std::endl;
    std::cout << "Hospital " << hosp_name << " has total capacity " << capacity << " and initial occupancy " << occupancy << std::endl;
    std::cout << "\n----------------------------\n" << std::endl;
    while(1) {
        // 3. UDP: recv query from the scheduler
        std::pair<int, std::string> query = hospitalB.recvUDPData(fd);
        std::string client_loc = query.second;
        if (client_loc == "Not Chosen") {
            continue;
        }
        std::cout << "Hospital" << HOSPITAL_NAME << " has received input from client at location " << client_loc << std::endl;
        // 4. Calculate score
        std::string msg;
        // 5. UDP: send score, distance to scheduler

        if (client_loc == hospitalB.location) {
            msg = hosp_port + " None None";
            std::cout << "Hospital " << hosp_name << " has capacity = " <<hospitalB.capacity<< ", occupation = "<<hospitalB.occupancy<<", availability= "<< hospitalB.calAvail() << std::endl;
            std::cout << "Hospital " << hosp_name << " has found the shortest path to client, distance = None" << std::endl;
            std::cout << "Hospital " << hosp_name << " has the score = None" << std::endl;             
            hospitalB.sendUDPdata(fd, msg, PORT_HOSPITAL);
            std::cout << "Hospital " << hosp_name << " has sent the score = None and distance = None to the Scheduler" << std::endl;
            
        }

        else if (hospitalB.checkValid(client_loc) == 0) {
            std::cout << "Hospital " << hosp_name << " has capacity = " <<hospitalB.capacity<< ", occupation = "<<hospitalB.occupancy<<", availability= "<< hospitalB.calAvail() << std::endl;
            double shortest = hospitalB.shortestDistance(client_loc, location);
            std::cout << "Hospital " << hosp_name << " has found the shortest path to client, distance = " << shortest << std::endl;
            double score = hospitalB.calScore(shortest);
            std::cout << "Hospital " << hosp_name << " has the score = " << score << std::endl;
            msg = hosp_port + " " + std::to_string(score) + " " + std::to_string(shortest);

            if (shortest < 0.0001) {
                msg = hosp_port + " None None";
            }
            else if (score < 0 || score > 1) {
                msg = hosp_port + " None " + std::to_string(shortest);
            } 
            
            hospitalB.sendUDPdata(fd, msg, PORT_HOSPITAL);
            std::cout << "Hospital " << hosp_name << " has sent the score = " << score << " and distance = " << shortest << " to the Scheduler" << std::endl;
        } 

        else {
            msg = hosp_port + " NOTFOUND NOTFOUND";
            std::cout << "Hospital " << hosp_name << " does not have the location " << client_loc << " in map" << std::endl;
            hospitalB.sendUDPdata(fd, msg, PORT_HOSPITAL);
            std::cout << "Hospital " << hosp_name << " has sent \"location not found\" to the Scheduler" << std::endl;
        }
        
        // 6. UDP: may recv update from scheduler - update
        std::pair<int, std::string> decision = hospitalB.recvUDPData(fd);
        if (decision.second == "Chosen") {
            //update
            hospitalB.occupancy ++;
            std::cout << "Hospital " << hosp_name << " has been assigned to a client, occupation is updated to " << hospitalB.occupancy << ", availability is updated to " << hospitalB.calAvail() << std::endl;
        }
        std::cout << "\n----------------------------\n" << std::endl;

    }

    close(fd);
    return EXIT_SUCCESS;

}