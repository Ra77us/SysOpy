#ifndef SHARED_H

#define SHARED_H

#include <unistd.h>
#include <signal.h>

#define STOP 1
#define DISCONNECT 2
#define LIST 3
#define CONNECT 4
#define MESSAGE 5
#define INIT 6

#define MAX_TEXT_SIZE 2048

#define PROJ_ID 2

typedef struct msgbuf {
    long mtype;
    char mtext[MAX_TEXT_SIZE];
    pid_t sender_pid;
    int sender_id;
} msgbuf;

typedef struct client {
    int id;
    int queue_id;
    int connected_to; // -1 if not connected
    pid_t pid;

} client;


const size_t MSG_SIZE = sizeof(msgbuf) - sizeof(long);

#endif
