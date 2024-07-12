
=> What I have completed:
   => I have done the code for the four source code files: client.cpp, serverA.cpp, serverB.cpp and serverM.cpp
   => I have tested the above codes on the input files a.txt and b.txt

=> What my code files are and what each one of them does.
   => My code files are serverM.cpp (for the main server), serverA.cpp and serverB.cpp (for the backend servers)
   => Also client.cpp (for the client server)
   => serverM.cpp: 
        => firstly reads the usernames sent to it from A to B. 
        => Next reads the client's input
        => Next determines which (if any) usernames don't exist
        => Next lets the clients know which (if any) usernames don't exist
        => Next splits the usernames into two sublists and sends it to A and B respectively
        => Receives the intersections from A and B
        => It runs an algorithm to determine the final time intersection
        => Sends the result to client
   => serverA.cpp and serverB.cpp
        => They read the inputs from the input data files
        => Next they send these to the main server
        => Only the names, and not the time intervals, are sent
        => Next, they receive the names from the main server
        => Then they run the algorithm to get the time intersections
        => Next, they send these intersections to the main server
   => client.cpp 
        => It takes the input from the user
        => It sends these names to the main server
        => It receives the names which do not exist
        => it receives the final time intervals
        => it restarts new requests

=> The format of all the messages exchanged
        => The usernames are character arrays
        => The time intervals are character arrays
        => All messages exchanged are hence character arrays (strings)
        => The username and time interval for a particular person is encapsulated in a linked list node
        => Intermediary data formats used are integer arrays and linked lists
