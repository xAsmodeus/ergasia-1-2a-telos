// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "bank_system.h" // For NUM_DEPARTMENTS

#define SERVER_PORT 8080
#define SERVER_ADDRESS "127.0.0.1"

// Function to send a request to the server and receive a response
void send_request(const char *request, char *response) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_ADDRESS, &server_addr.sin_addr) <= 0) {
        perror("Invalid server address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send request
    if (write(sockfd, request, strlen(request)) < 0) {
        perror("Write to server failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Receive response
    int n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("Read from server failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';

    strcpy(response, buffer);

    // Close socket
    close(sockfd);
}

int main() {
    int choice;
    char response[1024];
    char request[1024];

    while (1) {
        printf("\n--- Bank System Menu ---\n");
        printf("1. Display amount for an account number\n");
        printf("2. Add/subtract money from the amount of an account number\n");
        printf("3. Transfer money from one account to another\n");
        printf("4. Display the average of the amounts per account in a department with timestamp\n"); // Updated description
        printf("0. Exit\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) { // Input validation
            printf("Invalid input. Please enter a number.\n");
            // Clear input buffer
            while (getchar() != '\n');
            continue;
        }

        if (choice == 0) {
            printf("Exiting the program.\n");
            break;
        }

        int accountNumber1, accountNumber2;
        float amount;
        int department_number; // Variable for department number

        switch (choice) {
            case 1:
                printf("Enter account number: ");
                if (scanf("%d", &accountNumber1) != 1) {
                    printf("Invalid input.\n");
                    while (getchar() != '\n');
                    break;
                }
                snprintf(request, sizeof(request), "DISPLAY %d", accountNumber1);
                send_request(request, response);
                printf("Response: %s\n", response);
                break;

            case 2:
                printf("Enter account number: ");
                if (scanf("%d", &accountNumber1) != 1) {
                    printf("Invalid input.\n");
                    while (getchar() != '\n');
                    break;
                }
                printf("Enter amount to add/subtract (use negative value to subtract): ");
                if (scanf("%f", &amount) != 1) {
                    printf("Invalid input.\n");
                    while (getchar() != '\n');
                    break;
                }
                snprintf(request, sizeof(request), "MODIFY %d %.2f", accountNumber1, amount);
                send_request(request, response);
                printf("Response: %s\n", response);
                break;

            case 3:
                printf("Enter account number to transfer from: ");
                if (scanf("%d", &accountNumber1) != 1) {
                    printf("Invalid input.\n");
                    while (getchar() != '\n');
                    break;
                }
                printf("Enter account number to transfer to: ");
                if (scanf("%d", &accountNumber2) != 1) {
                    printf("Invalid input.\n");
                    while (getchar() != '\n');
                    break;
                }
                printf("Enter amount to transfer: ");
                if (scanf("%f", &amount) != 1) {
                    printf("Invalid input.\n");
                    while (getchar() != '\n');
                    break;
                }
                snprintf(request, sizeof(request), "TRANSFER %d %d %.2f", accountNumber1, accountNumber2, amount);
                send_request(request, response);
                printf("Response: %s\n", response);
                break;

            case 4:
                printf("Enter department number (0 to %d): ", NUM_DEPARTMENTS - 1);
                if (scanf("%d", &department_number) != 1) {
                    printf("Invalid input.\n");
                    while (getchar() != '\n');
                    break;
                }
                if (department_number < 0 || department_number >= NUM_DEPARTMENTS) {
                    printf("Invalid department number. Please enter a number between 0 and %d.\n", NUM_DEPARTMENTS - 1);
                    break;
                }
                snprintf(request, sizeof(request), "AVERAGE %d", department_number);
                send_request(request, response);
                
                // Parse the response to separate average and timestamp
                float avg;
                char timestamp[TIMESTAMP_SIZE];
                int scanned = sscanf(response, "Average balance: %f Timestamp: %[^\n]", &avg, timestamp);
                if (scanned == 2) {
                    printf("Average Balance: %.2f\n", avg);
                    printf("Timestamp: %s\n", timestamp);
                } else {
                    printf("Response: %s\n", response);
                }
                break;

            default:
                printf("Invalid choice. Please enter a valid option.\n");
        }
    }

    return 0;
}
