
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_STR_LEN 80
#define BUFFER_SIZE 1024
#define SUCCESS 0
#define ERROR 1

void cleanup_zombie(int sig) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        // Reap zombie processes
    }
    errno = saved_errno;
}

void handle_client_request(int client_socket, struct sockaddr_in client_info) {
    char data_buffer[BUFFER_SIZE];
    char response_buffer[BUFFER_SIZE];
    unsigned char operation_type, character;
    char input_data[MAX_STR_LEN + 1];

    // Print client IP and port
    char client_ip_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_info.sin_addr), client_ip_address, INET_ADDRSTRLEN);
    int client_port_number = ntohs(client_info.sin_port);

    while (1) {
        memset(data_buffer, 0, sizeof(data_buffer));
        ssize_t bytes_received = recv(client_socket, data_buffer, sizeof(data_buffer), 0);
        if (bytes_received <= 0) {
            printf("Exiting.. Received exit from client (%s, %d)\n", client_ip_address, client_port_number);
            break;
        }

        // Parse request
        operation_type = data_buffer[0];
        character = data_buffer[1];
        strncpy(input_data, data_buffer + 2, MAX_STR_LEN);
        input_data[MAX_STR_LEN] = '\0';

        printf("received \"%s\" from the client (%s, %d) with request No. %d\n",
               input_data, client_ip_address, client_port_number, operation_type);

        uint8_t status_code = SUCCESS;
        memset(response_buffer, 0, sizeof(response_buffer));

        switch (operation_type) {
            case 1: // Uppercase conversion
                for (int i = 0; input_data[i]; i++)
                    response_buffer[i] = toupper(input_data[i]);
                break;

            case 2: // Character count
                snprintf(response_buffer, sizeof(response_buffer), "%lu", strlen(input_data));
                break;

            case 3: // Frequency count of a character
                {
                    char target_character = character;
                    int count = 0;
                    for (int i = 0; input_data[i]; i++)
                        if (input_data[i] == target_character)
                            count++;
                    if (count == 0) {
                        status_code = ERROR;
                        snprintf(response_buffer, sizeof(response_buffer), "Character '%c' not found", target_character);
                    } else {
                        snprintf(response_buffer, sizeof(response_buffer), "%d", count);
                    }
                }
                break;

            default:
                status_code = ERROR;
                snprintf(response_buffer, sizeof(response_buffer), "Invalid operation");
                break;
        }

        // Send response
        send(client_socket, &status_code, 1, 0);
        send(client_socket, response_buffer, strlen(response_buffer), 0);
        printf("Sending \"%s\" to the client (%s, %d)\n", response_buffer, client_ip_address, client_port_number);
    }

    close(client_socket);
    exit(0);
}

int initialize_server_socket(int port_number) {
    int server_socket;
    struct sockaddr_in server_address;

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    // Reuse address
    int option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // Bind
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port_number);
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("bind error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_socket, 5) < 0) {
        perror("listen error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    return server_socket;
}

void manage_client_connections(int server_socket) {
    int client_socket;
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    while (1) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len)) < 0) {
            if (errno == EINTR)
                continue;
            else {
                perror("Accept error");
                exit(1);
            }
        }

        pid_t child_pid = fork();
        if (child_pid == 0) {
            // Child process
            close(server_socket);
            handle_client_request(client_socket, client_address);
        } else if (child_pid > 0) {
            // Parent process
            close(client_socket);
            printf("child with ID = %d has started.\n", child_pid);
        } else {
            perror("fork error");
            close(client_socket);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port_number = atoi(argv[1]);
    if (port_number < 1024) {
        printf("The port should be grater than 1024.\n");
        exit(EXIT_FAILURE);
    } else if (port_number > 65535) {
        printf("The port should be less than 65535.\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, cleanup_zombie); // Prevent zombie processes

    int server_socket = initialize_server_socket(port_number);
    printf("waiting for client messages.\n");

    manage_client_connections(server_socket);

    close(server_socket);
    return 0;
}

