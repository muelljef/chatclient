#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BSIZE 4096
#define HSIZE 12
#define MSIZE 512

void getInput(char *dest, size_t n, char *prompt);
int initiateContact(int argc, char *argv[]);
int sendMessage(char *handle, int sockfd);
int receiveMessage(int sockfd);

int main(int argc, char *argv[]) {
    int sockfd;
    char handle[HSIZE];

    sockfd = initiateContact(argc, argv);

    // Initialize the buffer to all nulls
    getInput(handle, HSIZE - 1, "What is your handle? (10 chars or less)");
    strcat(handle, ">");

    while (1) {

        if (sendMessage(handle, sockfd) == 0) {
            printf("Closing the connection and quitting\n");
            break;
        }

        if (receiveMessage(sockfd) == 0) {
            printf("Server closed the connection, quitting\n");
            break;
        }
    }

    close(sockfd);
    return 0;
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
    // Set all of destination to nulls clearing it
    memset(dest, '\0', n);
    // Copy up to the n - 1 of the input
    strncpy(dest, inputBuffer, n - 1);
}

int initiateContact(int argc, char *argv[])
{
    uint16_t portno;
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    // Check the command line parameters for hostname and port number are passed
    if (argc != 3) {
        fprintf(stderr,"usage %s hostname port#\n", argv[0]);
        exit(1);
    }

    // Convert and check that the port number given is in the valid range
    portno = (uint16_t) atoi(argv[2]);
    if (portno == 0) {
        fprintf(stderr, "ERROR, invalid port number, %u\n", portno);
        exit(1);
    }

    // Create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Error opening socket");
        exit(1);
    }

    // Get server details
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(1);
    }

    //Setting server details in struct serv_addr
    memset((char *) &serv_addr, '\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *) &serv_addr.sin_addr.s_addr, (char *) server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    //Connect to the socket
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Could not connect to port %u, chatclient closing\n", portno);
        exit(1);
    }

    return sockfd;
}

int sendMessage(char *handle, int sockfd)
{
    char message[501];
    char socketMessage[MSIZE];
    getInput(message, 501, handle);

    // Check if the client wants to quit
    if (strcmp(message, "\\quit") == 0) {
        return 0;
    }

    // Copy the handle and buffer message into the message var for sending
    memset(socketMessage, '\0', MSIZE);
    strncpy(socketMessage, handle, HSIZE - 1);
    strncat(socketMessage, message, 500);

    // Send the message to the server
    write(sockfd, socketMessage, MSIZE);

    return 1;
}

int receiveMessage(int sockfd)
{
    char buffer[BSIZE];

    // Set  the buffer to nulls
    memset(buffer, '\0', BSIZE);
    // Read the server response and print it
    if (read(sockfd, buffer, BSIZE - 1) == 0) {
        return 0;
    }
    printf("%s\n", buffer);

    return 1;
}
