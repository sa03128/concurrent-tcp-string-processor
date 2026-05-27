
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdint.h>

#define MAX_TEXT_LEN 80
#define BUF_SIZE 1024

int clientSocket; // Global so we can close it in the signal handler

void handle_sigquit(int signalNumber) {
    printf("\nExiting\n");
    close(clientSocket);
    exit(0);
}

void print_menu() {
    printf("Please select an operation from the following menu:\n");
    printf("1. Change the string to capital letters\n");
    printf("2. Count number of characters in the string\n");
    printf("3. Count the frequency of some character\n");
    printf("4. Enter another string\n");
}

void get_initial_string(char *clientText) {
    printf("Please enter a string (max 80 characters): ");
    fgets(clientText, MAX_TEXT_LEN + 1, stdin);
    clientText[strcspn(clientText, "\n")] = '\0'; // Remove newline
    if (strlen(clientText) == MAX_TEXT_LEN)
        while (getchar() != '\n');
}

void send_and_receive(unsigned char opCode, unsigned char opArg, char *clientText) {
    char recvBuffer[BUF_SIZE];
    int messageLength = 2 + strlen(clientText); // 1 byte opCode, 1 byte opArg, rest string
    char sendBuffer[BUF_SIZE];
    sendBuffer[0] = opCode;
    sendBuffer[1] = opArg;
    strcpy(sendBuffer + 2, clientText);

    printf("sending \"%s\" to the server with requested operation %d\n", clientText, opCode);
    send(clientSocket, sendBuffer, messageLength, 0);

    unsigned char responseCode;
    ssize_t bytesReceived = recv(clientSocket, &responseCode, 1, 0);
    if (bytesReceived <= 0) {
        printf("Server closed the connection.\n");
        exit(0);
    }

    ssize_t responseLen = recv(clientSocket, recvBuffer, BUF_SIZE - 1, 0);
    if (responseLen <= 0) {
        printf("Server closed the connection.\n");
        exit(0);
    }
    recvBuffer[responseLen] = '\0';

    if (responseCode == 0) {
        printf("Received \"%s\" from the server\n", recvBuffer);
    } else {
        printf("Error: %s\n", recvBuffer);
    }
}

void process_menu(char *clientText) {
    while (1) {
        print_menu();
        printf("Your choice: ");
        int userChoice;
        while (scanf("%d", &userChoice) != 1) {
            while (getchar() != '\n');   // Clear input buffer
            printf("Invalid choice. Please enter a valid integer> ");
        }
        getchar(); // clear newline from stdin

        unsigned char opCode = (unsigned char)userChoice;
        unsigned char opArg = 0;
        
        if (userChoice == 3) {
            printf("Enter the character to search for: ");
            char searchChar = getchar();
            getchar(); // consume newline
            opArg = (unsigned char)searchChar;
        } else if (userChoice == 4) {
            printf("Please enter a new string (max 80 characters): ");
            fgets(clientText, MAX_TEXT_LEN + 1, stdin);
            if (strlen(clientText) == MAX_TEXT_LEN)
                while (getchar() != '\n');
            clientText[strcspn(clientText, "\n")] = '\0';
            continue; // go back to menu
        }
        
        send_and_receive(opCode, opArg, clientText);
    }
}

void setup_connection(int argumentCount, char *argumentVector[], struct sockaddr_in *serverInfo) {
    if (argumentCount != 3) {
        fprintf(stderr, "Usage: %s <server IP> <server port>\n", argumentVector[0]);
        exit(EXIT_FAILURE);
    }
    if (atoi(argumentVector[2]) < 1024) {
        printf("The port should be grater than 1024.\n");
        exit(1);
    } else if (atoi(argumentVector[2]) > 65535) {
        printf("The port should be less than 65535.\n");
        exit(1);
    }
    
    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket error");
        exit(EXIT_FAILURE);
    }

    // Set server info
    memset(serverInfo, 0, sizeof(*serverInfo));
    serverInfo->sin_family = AF_INET;
    serverInfo->sin_port = htons(atoi(argumentVector[2]));
    inet_pton(AF_INET, argumentVector[1], &serverInfo->sin_addr);

    // Connect
    if (connect(clientSocket, (struct sockaddr *)serverInfo, sizeof(*serverInfo)) < 0) {
        perror("Connect error");
        exit(EXIT_FAILURE);
    }
}

int main(int argCount, char *argVector[]) {
    struct sockaddr_in serverInfo;
    char clientText[MAX_TEXT_LEN + 1];

    signal(SIGQUIT, handle_sigquit); // Register signal handler
    setup_connection(argCount, argVector, &serverInfo);
    get_initial_string(clientText);
    process_menu(clientText);

    close(clientSocket);
    return 0;
}
