#ifndef SHARED_H

#define SHARED_H

#include <unistd.h>
#include <mqueue.h>

#define STOP 6
#define DISCONNECT 5
#define LIST 4
#define CONNECT 3
#define MESSAGE 2
#define INIT 1

#define MAX_TEXT_SIZE 2048
#define MAX_MESSAGES 10

#define QUEUE_NAME_LEN 16
#define SERVER_QUEUE_NAME "/serv-que"

typedef struct client {
    int id;
    mqd_t queue;
    char queue_name[QUEUE_NAME_LEN];
    int connected_to; // -1 if not connected

} client;

#endif
