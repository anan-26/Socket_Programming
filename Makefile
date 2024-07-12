CC = g++
CFLAGS  = -g -Wall -w

# Define the targets and their dependencies
all: serverM serverA serverB client

serverM: serverM.cpp
	$(CC) $(CFLAGS) -o serverM serverM.cpp

serverA: serverA.cpp
	$(CC) $(CFLAGS) -o serverA serverA.cpp
	
serverB: serverB.cpp
	$(CC) $(CFLAGS) -o serverB serverB.cpp

client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp
	
clean:
	rm -f serverM serverA serverB client