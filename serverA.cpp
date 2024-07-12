#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

//defining the port numbers for the main server and backend server A
#define SERVER_M_PORT "23452"
#define SERVER_A_PORT "21452"

//structure to store names corresponding to a.txt
struct node
{
    char username[100];
    char time[100];
    struct node *next;
};

//structure to store the received names and time intervals
struct recvNode
{
    char recvUsername[100];
    char recvTime[100];
    struct recvNode *recvNext;
};

int main()
{
    using namespace std;

    FILE *fp;
    char line[100];
    struct node *head;
    struct node *newNode;
    struct node *temp;

    head = NULL;
    char fullLine[100];

    //reading from the file
    fp = fopen("a.txt", "r");
    if (fp == NULL)
    {
        printf("Error opening file.\n");
        exit(1);
    }

    //reading the file line by line
    while (fgets(line, 500, fp) != NULL)
    {
        newNode = (struct node *)malloc(sizeof(struct node));
        if (newNode == NULL)
        {
            printf("Error: Unable to allocate memory.\n");
            exit(1);
        }
        
        //converting the line to a string and preprocessing to remove any white spaces
        string str(line);
        str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());

        strcpy(line, str.c_str());
        sscanf(line, "%s", fullLine);
        
        //separating the username from the time interval
        int check = 0;
        int i = 0;
        char c;
        int pos = 0;
        while (check == 0)
        {
            c = fullLine[i];
            if (c == ';')
            {
                check = 1;
                pos = i;
            }
            i++;
        }

        for (int j = 0; j < pos; j++)
        {
            newNode->username[j] = fullLine[j];
            if (j + 1 == pos)
            {
                newNode->username[j + 1] = '\0';
            }
        }

        int k = 0;
        for (int j = pos + 1; fullLine[j] != '\0'; j++)
        {
            newNode->time[k] = fullLine[j];
            if (fullLine[j + 1] == '\0')
            {
                newNode->time[k + 1] = '\0';
            }
            k++;
        }

        newNode->next = NULL;

        if (head == NULL)
        {
            head = newNode;
            temp = head;
        }
        else
        {
            temp = head;
            while (temp->next != NULL)
            {
                temp = temp->next;
            }
            temp->next = newNode;
        }
    }

    fclose(fp);

    struct addrinfo hints, *mainInfo, *servAInfo;
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo("127.0.0.1", SERVER_M_PORT, &hints, &mainInfo);
    getaddrinfo("127.0.0.1", SERVER_A_PORT, &hints, &servAInfo);

    sockfd = socket(servAInfo->ai_family, servAInfo->ai_socktype, servAInfo->ai_protocol);
    
    //binding the server process to the socket
    bind(sockfd, servAInfo->ai_addr, servAInfo->ai_addrlen);
    cout << "Server A is up and running using UDP on port " << SERVER_A_PORT << "." << endl;

    int len;
    int bytes_sent;

    char *msgname;
    char *flag = "end";

    //sending names in backend server A to main server
    temp = head;
    while (temp != NULL)
    {
        msgname = temp->username;
        bytes_sent = sendto(sockfd, msgname, strlen(msgname),
                            MSG_CONFIRM, (const struct sockaddr *)mainInfo->ai_addr,
                            sizeof(*mainInfo->ai_addr));
        
        if (temp->next == NULL)
        {
            bytes_sent = sendto(sockfd, flag, strlen(flag),
                                MSG_CONFIRM, (const struct sockaddr *)mainInfo->ai_addr,
                                sizeof(*mainInfo->ai_addr));
        }
        temp = temp->next;
    }

    printf("Server A finished sending a list of usernames to Main Server.\n");

    //entering loop which loops every time the client program requests for new input
    while (true)
    {
        
        struct recvNode *recvHead;
        struct recvNode *recvNewNode;
        struct recvNode *recvTemp;
        recvHead = NULL;
        int valread;
        char recvname[100];
        for(int i =0; i<100; i++) {
            recvname[i] = '\0';
        }

        //receiving names from main, the end of which is indicated by the message "end"
        while (strcmp(recvname, "end") != 0)
        {
            recvNewNode = (struct recvNode *)malloc(sizeof(struct recvNode));

            for (int i = 0; i < 100; i++)
            {
                recvname[i] = 0;
            }

            valread = recvfrom(sockfd, recvname, 100, MSG_WAITALL, NULL, NULL);

            strcpy(recvNewNode->recvUsername, recvname);

            if (recvHead == NULL)
            {
                recvHead = recvNewNode;
                recvTemp = recvHead;
                recvTemp->recvNext = NULL;
            }
            else
            {
                recvTemp = recvHead;
                while (recvTemp->recvNext != NULL)
                {
                    recvTemp = recvTemp->recvNext;
                }
                recvTemp->recvNext = recvNewNode;
                recvNewNode->recvNext = NULL;
            }
        
        }

        printf("Server A received the usernames from Main Server using UDP over port %s\n", SERVER_A_PORT);
        recvTemp = recvHead;
    
        fflush(stdout);

        //counting the number of usernames received
        recvTemp = recvHead;
        int recvNodeCount = 0;
        if (strcmp(recvHead->recvUsername, "end") != 0)
        {
            while (recvTemp->recvNext!= NULL)
            {
                recvNodeCount++;
                recvTemp = recvTemp->recvNext;
            }
        }

        int timeArray[101];
        for (int i = 0; i<101; i++) {
            timeArray[i] = 0;
        }
        char number[100];
        for (int i = 0; i<100; i++) {
            number[i] = 0;
        }

        /* Algorithm to detect intersections
         * an integer array with 101 values is created
         * whenever an integer like 0 is detected in a range, the element corresponding to its index is incremented
         * for eg: [5,7] would have timeArray[5] and timeArray[6] incremented
         */
        int j;
        int i;
        int k, l;
        int next = 0;
        int prev = 0;
        bool alt = true;
        recvTemp = recvHead;
        if (strcmp(recvHead->recvUsername, "end") != 0)
        {
            while (recvTemp != NULL)
            {
                
                temp = head;
                
                while (temp != NULL)
                {
                    
                    if (strcmp(temp->username, recvTemp->recvUsername) == 0)
                    {
                        
                        i = 0;
                        
                        while (i < strlen(temp->time))
                        {
                            
                            if (temp->time[i] != '[' && temp->time[i] != ']' && temp->time[i] != ',' && temp->time[i] != ' ')
                            {
                                
                                number[0] = '\0';
                                j = 0;
                                
                                while (temp->time[i] != ',' && temp->time[i] != ']' && temp->time[i] != ' ')
                                {
                                    
                                    number[j] = temp->time[i];
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
                    }
                    temp = temp->next;
                }

                recvTemp = recvTemp->recvNext;
            }
        }
        
        char finalTimeArray[200] = "[";

        //building out the final result as a string
        i = 0;
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

        printf("Found the intersection result: %s\n", finalTimeArray);

        fflush(stdout);

        //sending the intersections to main server
        bytes_sent = sendto(sockfd, finalTimeArray, strlen(finalTimeArray),
                            MSG_CONFIRM, (const struct sockaddr *)mainInfo->ai_addr,
                            sizeof(*mainInfo->ai_addr));
        printf("Server A finished sending the response to Main Server.\n");
    }
    
    //closing the socket
    close(sockfd);
    return 0;
}
