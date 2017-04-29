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

    while (1) {
        // Initialize the buffer to all nulls
        memset(message, '\0', BSIZE);

        // Get the message from the client
        printf("Gypsy>");
        fgets(message, BSIZE, stdin);

        //search for newline and replace it will null char
        newLineChrPtr = strchr(message, '\n');
        if(newLineChrPtr != NULL) {
            *newLineChrPtr = '\0';
        }

        // Check if the client wants to quit
        if (strcmp(message, "\\quit") == 0) {
            printf("Closing the connection and quitting");
            break;
        }

        // Send the message to the server
        write(sockfd, message, sizeof(message));

        //Initialize the buffer to nulls
        memset(buffer, '\0', BSIZE);
        // Read the server response and print it
        if (read(sockfd, buffer, BSIZE - 1) == 0) {
            printf("Server closed the connection, quitting");
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
