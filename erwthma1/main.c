// main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void run_server() {
    if (fork() == 0) {
        execl("./server", "./server", (char *)NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    }
}

void run_client() {
    if (fork() == 0) {
        execl("./client", "./client", (char *)NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    }
}

int main() {
    run_server();
    sleep(1); // Give the server some time to start
    run_client();

    // Wait for child processes to finish
    wait(NULL);
    wait(NULL);

    return 0;
}
