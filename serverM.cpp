#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

//defining constant values for ports and backlog value
#define MYPORT_UDP "23452"
#define MYPORT_TCP "24452"
#define SERVER_B_PORT "22452"
#define SERVER_A_PORT "21452"
#define BACKLOG 10

//defining structures to store usernames for each backend server and creating sublists
struct nodeA
{
    char usernameA[100];
    struct nodeA *nextA;
};

struct nodeB
{
    char usernameB[100];
    struct nodeB *nextB;
};

struct nodeC
{
    char usernameC[200];
    struct nodeC *nextC;
};

struct nodesubA
{
    char usernamesubA[100];
    struct nodesubA *nextsubA;
};

struct nodesubB
{
    char usernamesubB[100];
    struct nodesubB *nextsubB;
};

//structure to store received usernames and times
struct recvNode
{
    char recvUsername[100];
    char recvTime[100];
    struct recvNode *recvNext;
};

int main()
{
    using namespace std;

    struct addrinfo hints, hints_tcp, *res, *res_tcp, *servBInfo, *servAInfo;
    int sockfd, sockfd_tcp;

    struct sockaddr_storage their_addr;
    socklen_t addr_size;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    memset(&hints_tcp, 0, sizeof hints_tcp);
    hints_tcp.ai_family = AF_UNSPEC;
    hints_tcp.ai_socktype = SOCK_STREAM;
    hints_tcp.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, MYPORT_UDP, &hints, &res);
    getaddrinfo(NULL, MYPORT_TCP, &hints_tcp, &res_tcp);
    getaddrinfo(NULL, SERVER_B_PORT, &hints, &servBInfo);
    getaddrinfo(NULL, SERVER_A_PORT, &hints, &servAInfo);

    //creating sockets for UDP and TCP connections
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    sockfd_tcp = socket(res_tcp->ai_family, res_tcp->ai_socktype, res_tcp->ai_protocol);

    //enabling socket reuse
    int reuse = 1;
    setsockopt(sockfd_tcp, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse));

    //bind sockets for the UDP connection
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    int bindtcp = bind(sockfd_tcp, res_tcp->ai_addr, res_tcp->ai_addrlen);
    if (bindtcp == -1)
    {
        printf("%s", strerror(errno));
        return 0;
    }

    cout << "Main Server is up and running." << endl;

    //listening to UDP connection
    int list, list_tcp;
    list = listen(sockfd, BACKLOG);

    int valread;
    int bytes_sent, bytes_sent_tcp;
    char buffer[100];
    for (int i = 0; i < 100; i++)
        buffer[i] = {0};

    int valread_tcp;

    //defining the head and temp nodes for structures
    struct nodeA *headA;
    struct nodeB *headB;
    struct nodeA *newNodeA;
    struct nodeA *tempA;
    struct nodeB *newNodeB;
    struct nodeB *tempB;
    headA = NULL;

    //taking names for backend server A
    while (true)
    {
        for (int i = 0; i < 100; i++)
        {
            buffer[i] = 0;
        }
        newNodeA = (struct nodeA *)malloc(sizeof(struct nodeA));
        struct sockaddr *src_addr;
        socklen_t *addrlen;
        valread = recvfrom(sockfd, buffer, 100, MSG_TRUNC, NULL, NULL);

        if (valread == -1)
        {
            cerr << "Error receiving data: " << strerror(errno) << endl;
        }
        printf("\n");
        strcpy(newNodeA->usernameA, buffer);

        newNodeA->usernameA[strlen(buffer)] = '\0';
        if (strcmp(newNodeA->usernameA, "end") == 0)
        {
            free(newNodeA);
            break;
        }

        newNodeA->nextA = NULL;
        if (headA == NULL)
        {
            headA = newNodeA;
            tempA = headA;
        }
        else
        {
            tempA = headA;
            while (tempA->nextA != NULL)
            {
                tempA = tempA->nextA;
            }
            tempA->nextA = newNodeA;
        }
    }
    
    printf("Main Server received the username list from server A using UDP over port %s\n", MYPORT_UDP);
    
    //taking names for backend server B
    headB = NULL;
    while (true)
    {

        for (int i = 0; i < 100; i++)
        {
            buffer[i] = 0;
        }
        
        newNodeB = (struct nodeB *)malloc(sizeof(struct nodeB));
        struct sockaddr *src_addr;
        socklen_t *addrlen;
        valread = recvfrom(sockfd, buffer, 100, MSG_TRUNC, NULL, NULL);
        
        if (valread == -1)
        {
            cerr << "Error receiving data: " << strerror(errno) << endl;
        }
        printf("\n");
        strcpy(newNodeB->usernameB, buffer);

        newNodeB->usernameB[strlen(buffer)] = '\0';
        if (strcmp(newNodeB->usernameB, "end") == 0)
        {
            free(newNodeB);
            break;
        }
        
        printf("\n");

        newNodeB->nextB = NULL;
        if (headB == NULL)
        {
            headB = newNodeB;
            tempB = headB;
        }
        else
        {
            tempB = headB;
            while (tempB->nextB != NULL)
            {
                tempB = tempB->nextB;
            }
            tempB->nextB = newNodeB;
        }
    }

    printf("Main Server received the username list from server B using UDP over port %s\n", MYPORT_UDP);

    //listening for TCP connection, backlog specified as 10
    list_tcp = listen(sockfd_tcp, BACKLOG);

    fflush(stdout);

    //creating new socket from accept for the TCP connection with client
    int newsockfd = accept(sockfd_tcp, NULL, NULL);

    //entering loop which will keep looping every time the client starts a new request
    while (true)
    {
        //structure to store names taken from the client
        struct nodeC *headC;
        struct nodesubA *headsubA = NULL;
        struct nodesubB *headsubB = NULL;

        struct nodeC *newNodeC;
        struct nodeC *tempC;

        struct nodesubA *newSubNodeA;
        struct nodesubA *tempsubA;

        struct nodesubB *newSubNodeB;
        struct nodesubB *tempsubB;

        struct recvNode *recvHead;
        struct recvNode *recvNewNode;
        struct recvNode *recvTemp;
        headC = NULL;
        
        //specifying variable which will store names given by client
        char clientInput[200];
        for (int i = 0; i < 200; i++)
            clientInput[i] = '\0';
        
        valread = recv(newsockfd, clientInput, 200, 0);
        clientInput[strlen(clientInput)] = '\0';
        printf("Main Server received the request from client using TCP over port %s\n", MYPORT_TCP);
        
        char name[100];
        for (int i = 0; i < 100; i++)
        {
            name[i] = '\0';
        }

        //recvNodeCount = 2 since we are considering two sublists: one for A and one for B
        int recvNodeCount = 2;
        int index = 0;

        newNodeC = (struct nodeC *)malloc(sizeof(struct nodeC));
        headC = NULL;
        
        //creating the client structure from the names given
        for (int i = 0; i <= strlen(clientInput); i++)
        {
        
            if (i != strlen(clientInput) && isalpha(clientInput[i]))
            {
                
                if (headC == NULL)
                {
                    headC = newNodeC;
                    headC->nextC = NULL;
                    tempC = headC;
                    
                }
                name[index++] = clientInput[i];
            }
            else
            {
                name[index] = '\0';
                index = 0;
                
                while (tempC->nextC != NULL)
                {
                    tempC = headC;
                    tempC = tempC->nextC;
                    
                }
                
                strcpy(tempC->usernameC, name);
                
                if (i != strlen(clientInput))
                {
                    newNodeC = (struct nodeC *)malloc(sizeof(struct nodeC));
                    tempC->nextC = newNodeC;
                    tempC = tempC->nextC;

                    for (int j = 0; j < 100; j++)
                        name[j] = 0;
                }
                tempC->nextC = NULL;
            }
        }

        fflush(stdout);
        
        //notFound will store those usernames which don't exist in either A or B
        int notFoundCount = 0;
        char notFound[100];
        for (int i = 0; i < 100; i++)
        {
            notFound[i] = '\0';
        }
        tempC = headC;

        fflush(stdout);

        /*
         * checking which (if any) names don't exist in either backend A or B
         * creating sublists for A and B from the usernames given
        */
        while (tempC != NULL)
        {
            //checking with the username list taken from backend server A
            tempA = headA;
            bool notfoundA = true, notfoundB = true;
            while (tempA != NULL)
            {
                
                newSubNodeA = (struct nodesubA *)malloc(sizeof(struct nodesubA));
               
                if (strcmp(tempC->usernameC, tempA->usernameA) == 0)
                {
                    notfoundA = false;
                    strcpy(newSubNodeA->usernamesubA, tempC->usernameC);
                    newSubNodeA->nextsubA = NULL;
                    if (headsubA == NULL)
                    {
                        headsubA = newSubNodeA;
                        tempsubA = headsubA;
                    }
                    else
                    {
                        tempsubA = headsubA;
                        while (tempsubA->nextsubA != NULL)
                        {
                            tempsubA = tempsubA->nextsubA;
                        }
                        tempsubA->nextsubA = newSubNodeA;
                    }
                    
                } 

                tempA = tempA->nextA;
                
            }

            //checking with the username list taken from backend server B
            tempB = headB;
            while (tempB != NULL)
            {

                newSubNodeB = (struct nodesubB *)malloc(sizeof(struct nodesubB));
                
                if (strcmp(tempC->usernameC, tempB->usernameB) == 0)
                {
                    notfoundB = false;
                    strcpy(newSubNodeB->usernamesubB, tempC->usernameC);
                    newSubNodeB->nextsubB = NULL;
                    if (headsubB == NULL)
                    {
                        headsubB = newSubNodeB;
                        tempsubB = headsubB;
                    }
                    else
                    {
                        tempsubB = headsubB;
                        while (tempsubB->nextsubB != NULL)
                        {
                            tempsubB = tempsubB->nextsubB;
                        }
                        tempsubB->nextsubB = newSubNodeB;
                    }

                   
                }

                tempB = tempB->nextB;
               
            }

            //if not found in both A or B, display as does not exist
            if (notfoundA && notfoundB)
            {
                notFoundCount++;
                if (notFoundCount > 1)
                {
                    strcat(notFound, ",");
                }
                strcat(notFound, tempC->usernameC);
            }

            tempC = tempC->nextC;
        }

        if (notFound[0] == '\0')
        {
            notFound[0] = ';';
        }

        //send client the list of names that don't exist
        bytes_sent_tcp = send(newsockfd, notFound, strlen(notFound), 0);
        char clientReplyForNotFound[10];
        recv(newsockfd, clientReplyForNotFound, 10, 0);

        if (notFound[1] != NULL)
        {
            printf("%s do not exist. Send a reply to the client.\n", notFound);
        }

        fflush(stdout);

        fflush(stdout);
        
        tempsubA = headsubA;
        
        //printing names located in server A and B respectively
        int keepCount = 0;
        if(tempsubA!=NULL) { printf("Found "); }
        while (tempsubA != NULL)
        {
            keepCount++;
            printf("%s%s", (keepCount > 1 ? ", " : ""), tempsubA->usernamesubA);
            tempsubA = tempsubA->nextsubA;
        }
        if (headsubA != NULL)
            printf(" located at server A. Send to server A.\n");

        tempsubB = headsubB;
        
        keepCount = 0;
        if(tempsubB!=NULL) { printf("Found "); }
        while (tempsubB != NULL)
        {
            keepCount++;
            printf("%s%s", (keepCount > 1 ? ", " : ""), tempsubB->usernamesubB);
            tempsubB = tempsubB->nextsubB;
        }
        if (headsubB != NULL)
            printf(" located at server B. Send to server B.\n");

        //sending backend server A the names in sublist A
        tempsubA = headsubA;
        char *flagname = "end";
        if (tempsubA == NULL)
        {
            bytes_sent = sendto(sockfd, flagname, strlen(flagname),
                                MSG_CONFIRM, (const struct sockaddr *)servAInfo->ai_addr,
                                sizeof(*servAInfo->ai_addr));
        }
        while (tempsubA != NULL)
        {
            fflush(stdout);

            bytes_sent = sendto(sockfd, tempsubA->usernamesubA, strlen(tempsubA->usernamesubA),
                                MSG_CONFIRM, (const struct sockaddr *)servAInfo->ai_addr,
                                sizeof(*servAInfo->ai_addr));

            if (bytes_sent == -1)
            {
                printf("Error: %s\n", strerror(errno));
            }

            if (tempsubA->nextsubA == NULL)
            {
                bytes_sent = sendto(sockfd, flagname, strlen(flagname),
                                    MSG_CONFIRM, (const struct sockaddr *)servAInfo->ai_addr,
                                    sizeof(*servAInfo->ai_addr));
            }

            tempsubA = tempsubA->nextsubA;
        }

        fflush(stdout);

        //sending backend server B the names in sublist B
        tempsubB = headsubB;
        //we identify the end of a sublist by a message "end", here flagname
        if (tempsubB == NULL)
        {
            bytes_sent = sendto(sockfd, flagname, strlen(flagname),
                                MSG_CONFIRM, (const struct sockaddr *)servBInfo->ai_addr,
                                sizeof(*servBInfo->ai_addr));
        }
        while (tempsubB != NULL)
        {
            fflush(stdout);

            bytes_sent = sendto(sockfd, tempsubB->usernamesubB, strlen(tempsubB->usernamesubB),
                                MSG_CONFIRM, (const struct sockaddr *)servBInfo->ai_addr,
                                sizeof(*servBInfo->ai_addr));
    
            if (tempsubB->nextsubB == NULL)
            {
                bytes_sent = sendto(sockfd, flagname, strlen(flagname),
                                    MSG_CONFIRM, (const struct sockaddr *)servBInfo->ai_addr,
                                    sizeof(*servBInfo->ai_addr));
            }

            tempsubB = tempsubB->nextsubB;
            
        }

        fflush(stdout);

        char finalArrayA[2][200];

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 200; j++)
            {
                finalArrayA[i][j] = '\0';
            }
        }

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        
        valread = recvfrom(sockfd, finalArrayA[0], 100, MSG_TRUNC, (struct sockaddr *)&client_addr, &client_addr_len);
        finalArrayA[0][strlen(finalArrayA[0])] = '\0';

        //if the sending port number belongs to A, print out A's intersection, else B's
        if (ntohs(client_addr.sin_port) == 21452)
        {
            printf("Main Server received from server A the intersection result using UDP over port %s:%s\n", MYPORT_UDP, finalArrayA[0]);
        }
        else if (ntohs(client_addr.sin_port) == 22452)
        {
            printf("Main Server received from server B the intersection result using UDP over port %s:%s\n", MYPORT_UDP, finalArrayA[1]);
        }

        fflush(stdout);
        valread = recvfrom(sockfd, finalArrayA[1], 100, MSG_TRUNC, (struct sockaddr *)&client_addr, &client_addr_len);
        finalArrayA[1][strlen(finalArrayA[1])] = '\0';
        //second recvfrom function, taking the input from whichever backend server sends next
        if (ntohs(client_addr.sin_port) == 21452)
        {
            printf("Main Server received from server A the intersection result using UDP over port %s:%s\n", MYPORT_UDP, finalArrayA[0]);
        }
        else if (ntohs(client_addr.sin_port) == 22452)
        {
            printf("Main Server received from server B the intersection result using UDP over port %s:%s\n", MYPORT_UDP, finalArrayA[1]);
        }

        fflush(stdout);

        //if A's or B's intersection is empty, final result is simply the other backend server's intersection
        char finalTimeArray[200] = "[";
        if (strcmp(finalArrayA[0], "[]") == 0)
        {
            strcpy(finalTimeArray, finalArrayA[1]);
        }
        else if (strcmp(finalArrayA[1], "[]") == 0)
        {
            strcpy(finalTimeArray, finalArrayA[0]);
        }

        /* Algorithm to detect intersections
         * an integer array with 101 values is created
         * whenever an integer like 0 is detected in a range, the element corresponding to its index is incremented
         * for eg: [5,7] would have timeArray[5] and timeArray[6] incremented
         */
        else
        {
            int timeArray[101];
            for (int i = 0; i < 101; i++)
            {
                timeArray[i] = 0;
            }
            char number[100] = "";
            int j;
            int i;
            int m = 0;
            int k = 0;
            int next = 0;
            int prev = 0;
            bool alt = true;
            
            while (m < 2)
            {
                i = 0;
                
                while (i < strlen(finalArrayA[m]))
                {
                   
                    if (finalArrayA[m][i] != '[' && finalArrayA[m][i] != ']' && finalArrayA[m][i] != ',' && finalArrayA[m][i] != ' ')
                    {
                        
                        number[0] = '\0';
                        j = 0;
                        
                        
                        while (finalArrayA[m][i] != ',' && finalArrayA[m][i] != ']')
                        {
                            
                            number[j] = finalArrayA[m][i];
                            j++;
                            i++;
                        }
                        number[j] = '\0';
                        
                        alt = !alt;
                        if (alt)
                        {
                            next = atoi(number);
                            for (k = prev; k < next; k++)
                            {
                                timeArray[k]++;
                            }
                        }
                        else
                        {
                            prev = atoi(number);
                        }
                    }
                    else
                    {
                        i++;
                    }
                }
                 
                m++;
                
            }

            fflush(stdout);

            i = 0;

            //building out the final result as a string
            char integer_string[50] = "";
            while (i < 101)
            {
                if (timeArray[i] == recvNodeCount)
                {
                    strcat(finalTimeArray, "[");
                    
                    while (i != 101 && timeArray[i] == recvNodeCount)
                    {
                        
                        if (i > 0 && timeArray[i - 1] != recvNodeCount)
                        {
                            sprintf(integer_string, "%d", i);
                            strcat(finalTimeArray, integer_string);
                        }
                        if (i != 100)
                        {
                            if (timeArray[i + 1] == recvNodeCount)
                            {
                                // strcat(finalTimeArray,",");
                            }

                            else
                            {
                                strcat(finalTimeArray, ",");
                                sprintf(integer_string, "%d", i + 1);
                                strcat(finalTimeArray, integer_string);
                                
                                strcat(finalTimeArray, "]");
                                strcat(finalTimeArray, ",");
                            }
                        }

                        i++;
                    }
                    i++;
                }
                else
                {
                    i++;
                }
            }
            if (strlen(finalTimeArray) > 1)
            {
                finalTimeArray[strlen(finalTimeArray) - 1] = ']';
            }
            else
            {
                finalTimeArray[strlen(finalTimeArray)] = ']';
            }
            finalTimeArray[strlen(finalTimeArray)] = '\0';

            printf("Found the intersection between the results from server A and B:%s\n", finalTimeArray);

            cout << endl;
        }

        fflush(stdout);

        //sending the final result to the client
        bytes_sent_tcp = send(newsockfd, finalTimeArray, strlen(finalTimeArray), 0);
        printf("Main Server sent the result to the client.\n");
        
    }

    //closing all sockets
    close(sockfd);
    shutdown(sockfd_tcp, SHUT_RDWR);
    close(sockfd_tcp);

    return 0;
}