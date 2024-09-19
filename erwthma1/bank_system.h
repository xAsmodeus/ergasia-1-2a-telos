// bank_system.h
#ifndef BANK_SYSTEM_H
#define BANK_SYSTEM_H

#include <time.h>

#define NUM_DEPARTMENTS 2     // Number of departments (e.g., A-E)
#define NUM_ACCOUNTS 500       // Number of accounts per department
#define NUM_REQUESTS 1000      // Number of load requests per department
#define TIMESTAMP_SIZE 20      // "YYYY-MM-DD HH:MM:SS"

typedef struct {
    int accountNumber;
    float amount;
    char timestamp[TIMESTAMP_SIZE]; // "YYYY-MM-DD HH:MM:SS"
} Record;

typedef struct {
    int record_count;
    Record records[NUM_ACCOUNTS];
} Department;

// Function declarations
void init_departments();
void generate_load_files(int department_number, const char *filename);
float display_amount(int accountNumber);
int modify_amount(int accountNumber, float amount);
int transfer_amount(int fromAccount, int toAccount, float amount);
float average_amount(int department_number);

#endif // BANK_SYSTEM_H
