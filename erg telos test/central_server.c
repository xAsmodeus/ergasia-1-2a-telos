#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "bank_system.h"

#define RECORDS_FILE "accounts.dat"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void lock_account(int accountNumber) {
    pthread_mutex_lock(&lock);
}

void unlock_account(int accountNumber) {
    pthread_mutex_unlock(&lock);
}

void handle_query(Request *request, Response *response) {
    FILE *file = fopen(RECORDS_FILE, "r+b");
    if (!file) {
        snprintf(response->message, sizeof(response->message), "Error opening file.");
        return;
    }

    fseek(file, 0, SEEK_SET);
    Account account;
    int found = 0;
    switch (request->queryType) {
        case QUERY_DISPLAY_AMOUNT:
            while (fread(&account, sizeof(Account), 1, file)) {
                if (account.accountNumber == request->queryDetails.accountNumber) {
                    snprintf(response->message, sizeof(response->message), "Amount: %.2f", account.amount);
                    found = 1;
                    break;
                }
            }
            if (!found) snprintf(response->message, sizeof(response->message), "Account not found.");
            break;
        case QUERY_UPDATE_AMOUNT:
            while (fread(&account, sizeof(Account), 1, file)) {
                if (account.accountNumber == request->queryDetails.accountNumber) {
                    account.amount += request->queryDetails.amount;
                    fseek(file, -sizeof(Account), SEEK_CUR);
                    fwrite(&account, sizeof(Account), 1, file);
                    snprintf(response->message, sizeof(response->message), "Amount updated.");
                    found = 1;
                    break;
                }
            }
            if (!found) snprintf(response->message, sizeof(response->message), "Account not found.");
            break;
        case QUERY_TRANSFER:
            Account accountB;
            while (fread(&account, sizeof(Account), 1, file)) {
                if (account.accountNumber == request->queryDetails.accountNumberA) {
                    account.amount -= request->queryDetails.amount;
                    fseek(file, -sizeof(Account), SEEK_CUR);
                    fwrite(&account, sizeof(Account), 1, file);
                    found = 1;
                    break;
                }
            }
            if (found) {
                fseek(file, 0, SEEK_SET);
                while (fread(&accountB, sizeof(Account), 1, file)) {
                    if (accountB.accountNumber == request->queryDetails.accountNumberB) {
                        accountB.amount += request->queryDetails.amount;
                        fseek(file, -sizeof(Account), SEEK_CUR);
                        fwrite(&accountB, sizeof(Account), 1, file);
                        snprintf(response->message, sizeof(response->message), "Transfer successful.");
                        found = 1;
                        break;
                    }
                }
            }
            if (!found) snprintf(response->message, sizeof(response->message), "Account not found.");
            break;
        case QUERY_AVERAGE:
            fseek(file, 0, SEEK_SET);
            float sum = 0.0;
            int count = 0;
            while (fread(&account, sizeof(Account), 1, file)) {
                if (account.departmentNumber == request->queryDetails.departmentNumber) {
                    sum += account.amount;
                    count++;
                }
            }
            if (count > 0) {
                snprintf(response->message, sizeof(response->message), "Average: %.2f, Timestamp: %ld", sum / count, time(NULL));
            } else {
                snprintf(response->message, sizeof(response->message), "No accounts found in department.");
            }
            break;
        default:
            snprintf(response->message, sizeof(response->message), "Invalid query type.");
            break;
    }
    fclose(file);
}

void *client_handler(void *arg) {
    Request request;
    Response response;
    int client_sock = *(int *)arg;
    free(arg);

    while (recv(client_sock, &request, sizeof(Request), 0) > 0) {
        handle_query(&request, &response);
        send(client_sock, &response, sizeof(Response), 0);
    }
    close(client_sock);
    return NULL;
}

int main() {
    int server_sock, client_sock, *new_sock;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    listen(server_sock, 3);

    while ((client_sock = accept(server_sock, (struct sockaddr *)&client, &client_len))) {
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
        if (pthread_create(&sniffer_thread, NULL, client_handler, (void *)new_sock) < 0) {
            perror("Thread creation failed");
            close(client_sock);
            continue;
        }
        pthread_detach(sniffer_thread);
    }

    if (client_sock < 0) {
        perror("Accept failed");
    }

    close(server_sock);
    return 0;
}
