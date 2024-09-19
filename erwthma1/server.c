// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include "bank_system.h"

#define PORT 8080

// Structure to pass client socket to thread
typedef struct {
    int client_socket;
} client_info_t;

// Function to handle client requests
void *handle_client(void *arg) {
    client_info_t *cinfo = (client_info_t *)arg;
    int client_socket = cinfo->client_socket;
    free(cinfo); // Free the allocated memory

    char buffer[1024];
    int n;

    while ((n = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0'; // Null-terminate the received data

        char response[1024];
        char command[256];
        int accountNumber1, accountNumber2;
        float amount;
        int department_number; // Variable for department number

        // Parse the command
        sscanf(buffer, "%s", command);

        if (strcmp(command, "DISPLAY") == 0) {
            if (sscanf(buffer, "%*s %d", &accountNumber1) != 1) {
                snprintf(response, sizeof(response), "Invalid DISPLAY command format. Usage: DISPLAY <account_number>");
            } else {
                float amt = display_amount(accountNumber1);
                if (amt < 0.0) {
                    snprintf(response, sizeof(response), "Account not found.");
                } else {
                    snprintf(response, sizeof(response), "%.2f", amt);
                }
            }
        }
        else if (strcmp(command, "MODIFY") == 0) {
            if (sscanf(buffer, "%*s %d %f", &accountNumber1, &amount) != 2) {
                snprintf(response, sizeof(response), "Invalid MODIFY command format. Usage: MODIFY <account_number> <amount>");
            } else {
                int result = modify_amount(accountNumber1, amount);
                if (result) {
                    float newAmount = display_amount(accountNumber1); 
                    snprintf(response, sizeof(response), "Amount updated successfully. New balance: %.2f", newAmount);
                } else {
                    snprintf(response, sizeof(response), "Failed to update amount. Account not found.");
                }
            }
        }
        else if (strcmp(command, "TRANSFER") == 0) {
            if (sscanf(buffer, "%*s %d %d %f", &accountNumber1, &accountNumber2, &amount) != 3) {
                snprintf(response, sizeof(response), "Invalid TRANSFER command format. Usage: TRANSFER <from_account> <to_account> <amount>");
            } else {
                int result = transfer_amount(accountNumber1, accountNumber2, amount);
                if (result) {
                    float newFromAmount = display_amount(accountNumber1); // New balance of from account
                    float newToAmount = display_amount(accountNumber2);   // New balance of to account
                    snprintf(response, sizeof(response), 
                             "Transfer successful.\nFrom Account New Balance: %.2f\nTo Account New Balance: %.2f", 
                             newFromAmount, newToAmount);
                } else {
                    snprintf(response, sizeof(response), "Transfer failed. Check account numbers and balances.");
                }
            }
        }
        else if (strcmp(command, "AVERAGE") == 0) {
            if (sscanf(buffer, "%*s %d", &department_number) != 1) {
                snprintf(response, sizeof(response), "Invalid AVERAGE command format. Usage: AVERAGE <department_number>");
            } else {
                float avg = average_amount(department_number);
                if (avg < 0.0) {
                    snprintf(response, sizeof(response), "Invalid department number.");
                } else {
                    // Get current timestamp
                    time_t t = time(NULL);
                    struct tm tm_info;
                    localtime_r(&t, &tm_info);
                    char timestamp[TIMESTAMP_SIZE];
                    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm_info);

                    snprintf(response, sizeof(response), "Average balance: %.2f Timestamp: %s", avg, timestamp);
                    
                    // Debugging: Log the response being sent
                    printf("Sending to client: %s\n", response);
                }
            }
        }
        else {
            snprintf(response, sizeof(response), "Invalid command.");
        }

        // Send response back to client
        if (write(client_socket, response, strlen(response)) < 0) {
            perror("Write to client failed");
            break;
        }
    }

    if (n < 0) {
        perror("Read error");
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;

    init_departments(); // Initialize the departments

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started. Listening on port %d...\n", PORT);

    while (1) {
        client_addr_len = sizeof(client_addr);
        // Accept a new connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Allocate memory for client info
        client_info_t *cinfo = malloc(sizeof(client_info_t));
        if (cinfo == NULL) {
            perror("Failed to allocate memory for client info");
            close(client_socket);
            continue;
        }
        cinfo->client_socket = client_socket;

        // Create a new thread to handle the client
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, (void *)cinfo) != 0) {
            perror("Failed to create thread");
            free(cinfo);
            close(client_socket);
            continue;
        }

        // Detach the thread to reclaim resources when it finishes
        pthread_detach(tid);
    }

    // Close the server socket (unreachable code in this example)
    close(server_socket);
    return 0;
}
