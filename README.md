# EE 450 Project - 21 spring in C++ 

* Name: Tieming Sun

## First thing to do 
**Change File Path**: for different "map.txt" file name, line 29 in `hospital.hpp` need to be changed 
```
#define FILE "map.txt"

=> #define FILE "XXXXX"
```
Then put the "map.txt" into the same directory as all code files.


## What I have done in this assignment


1. **Boot up Hospital and Scheduler:**
* Parse the map file into the hospital (A/B/C), and construct adjacancy list for map representation (unordered map data structure).
* Scheduler code will create UDP socket, and receive UDP datagram from hospital (A/B/C).
* Scheduler keep capacity & occupancy for each hospitals (map data structure).

2. **Client and Scheduler Forwarding Message:**
* Client send query to the Scheduler over TCP.
* Scheduler decides to send Client's query to Hospitals that still have available space.

3. **Scoring:**
* Scheduler construct message and send query to hospitals that are able to receive client over UDP.
* Hospitals judge if the client's location is legal and run Dijkstra algorithm to find shortest distance to the hospital.
* Hospitals calculate score based on shortest distance and availbility.
* Each Hospital sends score to the scheduler.

4. **Reply Message to Client:**
* Scheduler select the highest score to the client. If tie, choose the lowest distance.
* Scheduler sends reply the optimal hospital to the client, also sends the decision to all hospitals.
* The chosen hospital and Scheuler update the occupancy.

## Code Files
Totally there are 7 code files in 3 types: client, scheduler and hospital

### **`client.cpp`: Serve as TCP client**
1. Create a message based on parameter that is received.
2. Connect with the scheduler over TCP, using a random port number.
3. Send the location message and receive the allocated hospital name from the scheduler.

### **`scheduler.cpp, scheduler.hpp`: Serve as TCP server and UDP server**

> A class called `Scheduler` is defined in `scheduler.hpp` file, and main function runs in `scheduler.cpp`.

0. **Loop in 2 - 6** 
1. Create a UDP socket and receive datagrams from hospitals (A/B/C), and store current location, capacity and occupancy in scheduler. Create a TCP socket. 
2. Receive client location from Client.
3. Send client location to hospitals that have available space.
4. Receive score and distance from the Hospitals.
5. Choose the best score & distance and send decision to client and all hospitals.
6. Update the chosen hospital's occupancy.

### `hospital.hpp, hospitalA.cpp, hospitalB.cpp, hospitalC.cpp`: Serve as UDP client

> A class called `Hospital` is defined in `hospital.hpp` file, and main function runs in `hospitalA.cpp hospitalB.cpp hosptialC.cpp`.

0. **Loop in 3 - 6**
1. Read map file and parse the file as an adjacancy list to represent undirected graph.
2. Create a UDP socket and send to the Scheduler, with current location, capacity and occupancy.
3. Receive a UDP socket that contains client location from Scheduler.
4. Calculate availability, shortest Distance and final score.
5. Send score and shortest Distance to the Scheduler.
6. Receive the final decision from the Scheduler, and update occupancy if it is chosen.


## Format of message exchanged

### Between Client and Scheduler

* Client send the location to the Scheduler in char*[] type. eg. "18".
* Scheduler sends decision to Client in char*[] type. Types are = "A", "B", "C", "None", "NOTFOUND", "NOAssignment".

### Between Scheduler and Hospital

* Hospital send the boot up message separated in a single space, in char*[] type. message is "hospital_port location capacity occupancy" eg. "30942 11 20 15".
* Scheduler send the location to Hospitals that are availability > 0 in char*[] type. eg. "18".
* Hospital send score and distance to Scheduler separated in a single space, in char*[] type. message is "hospital_port score distance" eg. "30942 0.1184242 784.1325258".
* Scheduler send the decision message in char*[] type. Types are = "Chosen", "Not Chosen".

## Idiosyncrasy

* Illegal hospital / client input parameter eg. hospital location = -1. In this case, the program will says "Please input valid parameter" and quit.
* Hospital location is the same as Client location and the hospital still have availability. In this case, distance = 0 and therefore score = None. 


## Reused Code

* I reuse APIs and error detections that are from Beej's web socket tutorial.

