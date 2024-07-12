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

//defining the port numbers and backlog
#define MAIN_SERVER_PORT "24452"
#define BACKLOG 10

//defining a function called filter names to separate the names which do not exist from others
void filterNames(char *first, char *second, char *filterednames)
{
   
    char first_list[20][100];

    char second_list[20][100];
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            first_list[i][j] = '\0';
            second_list[i][j] = '\0';
        }
    }

    char first_modified[2000] = "";
    strcat(first_modified, first);
    strcat(first_modified, " ");

    char second_modified[2000] = "";
    strcat(second_modified, second);
    strcat(second_modified, ",");
    
    int nameBegin = 0;
    int names1C = 0;
    for (int i = 0; i < strlen(first_modified); i++)
    {
        if (first_modified[i] == ' ')
        {
            strncpy(first_list[names1C], first_modified + nameBegin, i - nameBegin);
            nameBegin = i + 1;
            names1C++;
        }
    }

    nameBegin = 0;
    int names2C = 0;
    for (int i = 0; i < strlen(second_modified); i++)
    {
        if (second_modified[i] == ',')
        {
            strncpy(second_list[names2C], second_modified + nameBegin, i - nameBegin);
            nameBegin = i + 1;
            names2C++;
        }
    }

    for (int i = 0; i < 2000; i++)
    {
        filterednames[i] = '\0';
    }
    int filterc = 0;
    for (int i = 0; i < names1C; i++)
    {
        bool found = false;
        for (int j = 0; j < names2C; j++)
        {
            if (strcmp(first_list[i], second_list[j]) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {

            if (filterc > 0)
            {
                strcat(filterednames, ",");
            }
            strcat(filterednames, first_list[i]);
            filterc++;
        }
    }
}

int main()
{

    using namespace std;

    struct addrinfo hints, *servMInfo;
    int sockfd;
    int bytes_sent;
    char buffer[1024] = {0};

    struct sockaddr_storage their_addr;
    socklen_t addr_size;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo("127.0.0.1", MAIN_SERVER_PORT, &hints, &servMInfo);

    sockfd = socket(servMInfo->ai_family, servMInfo->ai_socktype, servMInfo->ai_protocol);
    
    cout << "Client is up and running." << endl;

    int cval = connect(sockfd, servMInfo->ai_addr, servMInfo->ai_addrlen);

    //entering the loop which will loop every time the client program requests for new input
    while (true)
    {

        if (cval == -1)
        {
            printf("connect: %d %s\n", cval, strerror(errno));
        }

        char input[1024];
        for (int i = 0; i < 1024; i++)
        {
            input[i] = '\0';
        }
        
        printf("Please enter the usernames to check schedule availability:");
        scanf("%[^\n]s", input);
        //sending the input from the user to the main server
        bytes_sent = send(sockfd, input, strlen(input), 0);
        printf("Client finished sending the usernames to Main Server.\n");

        fflush(stdout);

        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        if (getsockname(sockfd, (sockaddr *)&clientAddr, &clientAddrLen) == -1)
        {
            std::cerr << "Failed to get socket name\n";
            close(sockfd);
            return 1;
        }
        
        char recvnotFound[100];
        int valread = 0;
        for (int i = 0; i < 100; i++)
        {
            recvnotFound[i] = '\0';
        }

        valread = recv(sockfd, recvnotFound, 1000, 0);

        //displaying the names that do not exist
        if (recvnotFound[0] != ';')
        {
            printf("Client received the reply from Main Server using TCP over port %d: %s do not exist.\n", ntohs(clientAddr.sin_port), recvnotFound);
        }
        //sending an acknowledgment to the main server after it sends the list of names that don't exist
        send(sockfd, (char *)"OK", 3, 0);

        char finalResult[100];
        for (int i = 0; i < 100; i++)
        {
            finalResult[i] = '\0';
        }
        //receiving the final intersections from the main server
        valread = recv(sockfd, finalResult, 1000, 0);
        char filterednames[2000] = "";
        
        //separating the names that do not exist from the final list
        filterNames(input, recvnotFound, filterednames);
        if (strlen(filterednames) > 0) {
            printf("Client received the reply from Main Server using TCP over port %d: Time intervals %s works for %s.\n", ntohs(clientAddr.sin_port), finalResult, filterednames);
        }

        //asking user to enter again
        printf("\nStarting new request\n---------------\n");
        getc(stdin);
    }

    //closing all sockets
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    return 0;
}