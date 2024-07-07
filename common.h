#include <stdio.h>
#include <string.h>

#define CONNESSIONI_MAX  10
#define SERVER_ADDRESS  "127.0.0.1"
#define SERVER_PORT     2024
#define QUIT_COMM      "quit"

#define handle_error(msg)           do { perror(msg); exit(EXIT_FAILURE); } while (0)
