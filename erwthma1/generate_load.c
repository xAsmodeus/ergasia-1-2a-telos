#include "bank_system.h"
#include <stdio.h>

int main() {
    init_departments(); // Initialize departments with 500 records each

    for (int i = 0; i < NUM_DEPARTMENTS; i++) {
        char filename[256];
        snprintf(filename, sizeof(filename), "load_department_%c.txt", 'A' + i);
        generate_load_files(i, filename);
        printf("Generated load file for department %c: %s\n", 'A' + i, filename);
    }

    return 0;
}
