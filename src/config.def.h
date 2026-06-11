#ifndef CONFIG_H
#define CONFIG_H
#define PORT_MAX     65535 // maximum port size
#define PORT_MIN     1024  // minimum port size

/* Server settings */
#define DEFAULT_PORT 6060
#define BUFFER_SIZE  512   // Misc buffer size
#define MAX_MSG_SIZE 512   // max string length of messages sent to the server
#define MAX_THREADS  10    // max amount of connections
#define BACKLOG_SIZE 10    // amount of items available in the backlog
#define THREAD_CHECKS 1    // amount of checks that threads make per second
#endif
