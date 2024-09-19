#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "bank_system.h"

void send_request(int sock, Request *request) {
    send(sock, request, sizeof(Request), 0);
}

void receive_response(int sock, Response *response) {
    recv(sock, response, sizeof(Response), 0);
}

void print_menu() {
    printf("Select query type:\n");
    printf("1: Display amount by account number\n");
    printf("2: Update amount by account number\n");
    printf("3: Transfer amount between accounts\n");
    printf("4: Display average amount in department\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    Request request;
    Response response;
    int query_type;
    printf("Welcome to the bank system.\n");

    print_menu();
    scanf("%d", &query_type);

    request.queryType = query_type;

    switch (query_type) {
        case QUERY_DISPLAY_AMOUNT:
        case QUERY_UPDATE_AMOUNT:
            printf("Enter account number: ");
            scanf("%d", &request.queryDetails.accountNumber);
            if (query_type == QUERY_UPDATE_AMOUNT) {
                printf("Enter amount to add/subtract: ");
                scanf("%f", &request.queryDetails.amount);
            }
            break;
        case QUERY_TRANSFER:
            printf("Enter source account number: ");
            scanf("%d", &request.queryDetails.accountNumberA);
            printf("Enter destination account number: ");
            scanf("%d", &request.queryDetails.accountNumberB);
            printf("Enter amount to transfer: ");
            scanf("%f", &request.queryDetails.amount);
            break;
        case QUERY_AVERAGE:
            printf("Enter department number: ");
            scanf("%hhu", &request.queryDetails.departmentNumber);
            break;
        default:
            printf("Invalid query type.\n");
            close(sock);
            return 1;
    }

    send_request(sock, &request);
    receive_response(sock, &response);

    printf("Response: %s\n", response.message);

    close(sock);
    return 0;
}
