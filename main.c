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

#define BSIZE 4096
#define HSIZE 12
#define MSIZE 512

void error(const char *msg);
void getInput(char *dest, size_t n, char *prompt);

int main() {
    uint16_t portno;
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BSIZE];
    char message[501];
    char socketMessage[MSIZE];
    char handle[HSIZE];
    char portNumber[6];
    char hostName[16];

    // The hardcoded port number to talk to the chatserver on
    portno = 50517;

    getInput(hostName, 16, "Enter the host name: ");
    getInput(portNumber, 6, "Enter the port number: ");

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
        fprintf(stderr, "Could not connect to port %u, chatclient closing\n", portno);
        exit(2);
    }

    // Initialize the buffer to all nulls
    getInput(handle, HSIZE - 1, "What is your handle? (10 chars or less)");
    strcat(handle, ">");

    while (1) {
        getInput(message, 501, handle);

        // Check if the client wants to quit
        if (strcmp(message, "\\quit") == 0) {
            printf("Closing the connection and quitting\n");
            break;
        }

        // Copy the handle and buffer message into the message var for sending
        memset(socketMessage, '\0', MSIZE);
        strncpy(socketMessage, handle, HSIZE - 1);
        strncat(socketMessage, message, 500);

        // Send the message to the server
        write(sockfd, socketMessage, MSIZE);

        // Set  the buffer to nulls
        memset(buffer, '\0', BSIZE);
        // Read the server response and print it
        if (read(sockfd, buffer, BSIZE - 1) == 0) {
            printf("Server closed the connection, quitting\n");
            break;
        }
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

void getInput(char *dest, size_t n, char *prompt)
{
    char inputBuffer[BSIZE];
    char *newLineChrPtr;

    // Initialize the buffer to all nulls
    memset(inputBuffer, '\0', BSIZE);
    // Get the message from the client
    printf("%s", prompt);
    fgets(inputBuffer, BSIZE, stdin);
    //search for newline and replace it will null char
    newLineChrPtr = strchr(inputBuffer, '\n');
    if(newLineChrPtr != NULL) {
        *newLineChrPtr = '\0';
    }
    // Copy the first 10 characters for the handle into the handle parameter
    memset(dest, '\0', n);
    strncpy(dest, inputBuffer, n - 1);
}