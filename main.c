#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BSIZE 501

void error(const char *msg);
void removeNewline(char *buffer, int *n);

int main() {
    uint16_t portno;
    int sockfd, recMessageSize, quitBool;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BSIZE];
    char message[BSIZE];
    char *newLineChrPtr;

    // The hardcoded port number to talk to the chatserver on
    portno = 50517;

    // Create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error opening socket");
    }

    //Get server details
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    //Setting server details in struct serv_addr
    memset((char *) &serv_addr, '\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *) &serv_addr.sin_addr.s_addr, (char *) server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    //Connect to the socket
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "Could not connect to port %u, otp_enc closing\n", portno);
        exit(2);
    }

    quitBool = 0;
    while (quitBool == 0) {
        //Initialize the buffer to nulls
        memset(message, '\0', BSIZE);
        printf("Gypsy>");
        fgets(message, BSIZE, stdin);
        //search for newline (from just before EOF) and remove it
        newLineChrPtr = strchr(message, '\n');
        if(newLineChrPtr != NULL) {
            *newLineChrPtr = '\0';
        }

        if (strcmp(message, "\\quit") == 0) {
            quitBool = 1;
            continue;
        }

        write(sockfd, message, sizeof(message));

        //Initialize the buffer to nulls
        memset(buffer, '\0', BSIZE);
        // Read the server response and print it
        read(sockfd, buffer, BSIZE - 1);
        printf("%s\n", buffer);
    }

    close(sockfd);
    return 0;
}

//entry: error string such as calling function error
//exit: will report msg with errno and exit with status 1
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

//entry: string and string length of n
//exit: first newline will be replace by null terminating ch
//      and n will be decremented
void removeNewline(char *buffer, int *n)
{
    int i;
    for(i = 0; i < *n ; i++)
    {
        if(buffer[i] == '\n')
        {
            buffer[i] = '\0';
            *n--;
        }
    }
}