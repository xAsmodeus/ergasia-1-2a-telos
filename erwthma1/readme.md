gcc -o server server.c bank_system.c -pthread
gcc -o client client.c
gcc -o generate_load generate_load.c bank_system.c
./generate_load
./server
./client