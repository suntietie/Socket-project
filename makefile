CC=g++
CFLAGS=-DDEBUG -std=c++11

all: scheduler hospitalA hospitalB hospitalC client

scheduler : scheduler.cpp 
	$(CC) $(CFLAGS) -o scheduler scheduler.cpp

hospitalA : hospitalA.cpp hospital.hpp
	$(CC) $(CFLAGS) -o hospitalA hospitalA.cpp

hospitalB : hospitalB.cpp hospital.hpp
	$(CC) $(CFLAGS) -o hospitalB hospitalB.cpp

hospitalC : hospitalC.cpp hospital.hpp
	$(CC) $(CFLAGS) -o hospitalC hospitalC.cpp	

client : client.cpp
	$(CC) $(CFLAGS) -o client client.cpp

clean:
	$(RM) scheduler hospitalA hospitalB hospitalC client