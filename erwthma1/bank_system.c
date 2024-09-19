// bank_system.c
#include "bank_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

Department departments[NUM_DEPARTMENTS];

// Initialize departments with random data
void init_departments() {
    srand(time(NULL));

    for (int i = 0; i < NUM_DEPARTMENTS; i++) {
        departments[i].record_count = 0;
        for (int j = 0; j < NUM_ACCOUNTS; j++) {
            departments[i].records[j].accountNumber = i * NUM_ACCOUNTS + j + 1;
            departments[i].records[j].amount = (float)(rand() % 100000) / 100; // $0.00 to $999.99

            time_t t = time(NULL);
            struct tm tm_info = *localtime(&t);
            int n = snprintf(departments[i].records[j].timestamp, sizeof(departments[i].records[j].timestamp),
                     "%04d-%02d-%02d %02d:%02d:%02d",
                     tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday,
                     tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec);
            // Ensure timestamp is not truncated
            if (n >= sizeof(departments[i].records[j].timestamp)) {
                fprintf(stderr, "Timestamp truncated for account %d in department %d\n",
                        departments[i].records[j].accountNumber, i);
            }
            departments[i].record_count++;
        }
    }
}

// Generate load files with random requests for a specific department
void generate_load_files(int department_number, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    for (int i = 0; i < NUM_REQUESTS; i++) {
        int queryType;
        float randPercent = (float)rand() / RAND_MAX;
        if (randPercent < 0.35) {
            queryType = 1; // DISPLAY
        } else if (randPercent < 0.70) {
            queryType = 2; // MODIFY
        } else if (randPercent < 0.95) {
            queryType = 3; // TRANSFER
        } else {
            queryType = 4; // AVERAGE
        }

        int accountNumber = rand() % (NUM_DEPARTMENTS * NUM_ACCOUNTS) + 1;
        float amount = (float)(rand() % 100000) / 100; // $0.00 to $999.99
        int fromAccount = rand() % (NUM_DEPARTMENTS * NUM_ACCOUNTS) + 1;
        int toAccount = rand() % (NUM_DEPARTMENTS * NUM_ACCOUNTS) + 1;

        // 80% chance to select an account within the specified department for DISPLAY and MODIFY
        if (queryType == 1 || queryType == 2) {
            float prob = (float)rand() / RAND_MAX;
            if (prob < 0.8) {
                accountNumber = department_number * NUM_ACCOUNTS + (rand() % NUM_ACCOUNTS) + 1;
            }
        }

        if (queryType == 1) {
            fprintf(file, "%d DISPLAY %d\n", department_number, accountNumber);
        } else if (queryType == 2) {
            fprintf(file, "%d MODIFY %d %.2f\n", department_number, accountNumber, amount);
        } else if (queryType == 3) {
            fprintf(file, "%d TRANSFER %d %d %.2f\n", department_number, fromAccount, toAccount, amount);
        } else if (queryType == 4) {
            fprintf(file, "%d AVERAGE\n", department_number);
        }
    }

    fclose(file);
}

// Display the amount for a given account number
float display_amount(int accountNumber) {
    for (int i = 0; i < NUM_DEPARTMENTS; i++) {
        for (int j = 0; j < departments[i].record_count; j++) {
            if (departments[i].records[j].accountNumber == accountNumber) {
                return departments[i].records[j].amount;
            }
        }
    }
    return -1; // Account not found
}

// Modify the amount for a given account number
int modify_amount(int accountNumber, float amount) {
    for (int i = 0; i < NUM_DEPARTMENTS; i++) {
        for (int j = 0; j < departments[i].record_count; j++) {
            if (departments[i].records[j].accountNumber == accountNumber) {
                departments[i].records[j].amount += amount;
                // Update timestamp
                time_t t = time(NULL);
                struct tm tm_info = *localtime(&t);
                snprintf(departments[i].records[j].timestamp, sizeof(departments[i].records[j].timestamp),
                         "%04d-%02d-%02d %02d:%02d:%02d",
                         tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday,
                         tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec);
                return 1; // Success
            }
        }
    }
    return 0; // Account not found
}

// Transfer amount from one account to another
int transfer_amount(int fromAccount, int toAccount, float amount) {
    float fromAmount = display_amount(fromAccount);
    if (fromAmount < amount) {
        return 0; // Insufficient funds
    }
    if (modify_amount(fromAccount, -amount) && modify_amount(toAccount, amount)) {
        return 1; // Success
    }
    return 0; // Failure
}

// Calculate the average amount for a specific department
float average_amount(int department_number) {
    if (department_number < 0 || department_number >= NUM_DEPARTMENTS) {
        return -1.0; // Indicates invalid department number
    }

    int total_accounts = departments[department_number].record_count;
    if (total_accounts == 0) {
        return 0.0;
    }

    float total_amount = 0.0;
    for (int j = 0; j < departments[department_number].record_count; j++) {
        total_amount += departments[department_number].records[j].amount;
    }

    return total_amount / total_accounts;
}
