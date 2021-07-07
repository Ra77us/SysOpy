#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <mqueue.h>
#include <time.h>

int client_id = -1;
char queue_name[QUEUE_NAME_LEN];
mqd_t client_queue;
mqd_t connected_to_queue = -1;
mqd_t server_queue;
int connected_to = -1;

void print_error(char *text, int with_errno) {
    if (with_errno == 1) {
        fprintf(stderr, "%s: %s \n", text, strerror(errno));
    } else {
        fprintf(stderr, "%s\n", text);
    }
    exit(-1);
}

void send_msg(mqd_t queue, unsigned int type, char *msg) {
    if ((mq_send(queue, msg, strlen(msg), type)) == -1) {
        print_error("Error while sending message from client\n", 1);
    }
}

void get_msg(mqd_t queue, unsigned int *type, char *msg) {
    if ((mq_receive(queue, msg, MAX_TEXT_SIZE, type)) == -1)
        print_error("Error while receiving message in client \n", 1);
}

void SIGINT_handler(int signo) {
    printf("SIGINT received. Shutting down client\n");
    exit(0);
}

void setup_notifications() { 
    struct sigevent event;
    event.sigev_signo = SIGRTMIN;
    event.sigev_notify = SIGEV_SIGNAL;

    if (mq_notify(client_queue, &event) == -1)
        print_error("Error while setting notifications", 1);

}


void message_handler(int signum) { 
    char *message = calloc(MAX_TEXT_SIZE + 1, sizeof(char));
    unsigned int type = -1;
    get_msg(client_queue, &type, message);
    int connected_id;
    char connected_name[25];

    switch (type) {
        case STOP:
            exit(0);
            break;
        case DISCONNECT:
            connected_to = -1;
            connected_to_queue = -1;
            printf("Disconnected\n");
            break;
        case CONNECT:
            sscanf(message, "%d %s", &connected_id, connected_name);

            if (connected_id == -1) {
                printf("Connection failed!\n");
                break;
            }
            connected_to = connected_id;
            if ((connected_to_queue = mq_open(connected_name, O_WRONLY)) == -1)
                print_error("Error while getting client queue", 1);
            printf("Connected with %d\n", connected_to);
            break;
        case LIST:
            printf("%s", message);
            break;
        case MESSAGE:
            printf("%d says: %s", connected_to, message);
            break;
        default:
            print_error("Unknown message type in server queue", 0);
            break;
    }
    setup_notifications();
    free(message);
}

void disconnect_action() { 
    if (connected_to_queue == -1) {
        printf("Already disconnected \n");
        return;
    }

    char text[10];
    sprintf(text, "%d", client_id);
    send_msg(server_queue, DISCONNECT, text);

    connected_to = -1;
    connected_to_queue = -1;
    printf("Disconnected\n");
}

void stop_action() { 
    if (connected_to != -1)
        disconnect_action();
    char text[10];
    sprintf(text, "%d", client_id);
    send_msg(server_queue, STOP, text);

    mq_close(client_queue);
    mq_unlink(queue_name);
    mq_close(server_queue);


    printf("Stopped\n");
}


void join_server() { 

    queue_name[0] = '/';
    for (int i = 1; i < QUEUE_NAME_LEN - 2; i++)
        queue_name[i] = rand() % ('z' - 'a' + 1) + 'a';

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_TEXT_SIZE;
    if ((client_queue = mq_open(queue_name, O_EXCL | O_RDWR | O_CREAT, 0666, &attr, &attr)) == -1)
        print_error("Error while creating queue", 1);

    if ((server_queue = mq_open(SERVER_QUEUE_NAME, O_WRONLY)) == -1)
        print_error("Error while getting server queue", 1);

    char *reply = calloc(10, sizeof(char));
    unsigned int type;
    send_msg(server_queue, INIT, queue_name);
    get_msg(client_queue, &type, reply);
    if (type != INIT) {
        printf("Cannot join server - server full!\n");
        exit(0);
    }
    client_id = atoi(reply);
    printf("Succesfully joined server. Client ID: %d\n", client_id);
    free(reply);
}

void list_action() { 
    char text[10];
    sprintf(text, "%d", client_id);
    send_msg(server_queue, LIST, text);
}

void connect_action(int id) { 
    if (id == client_id) {
        printf("Cannot connect with yourself\n");
        return;
    }
    if (connected_to != -1) {
        printf("Disconnect first\n");
        return;
    }

    char text[10];
    sprintf(text, "%d %d", client_id, id);
    send_msg(server_queue, CONNECT, text);
    printf("Connecting to %d\n", id);
}


void msg_action(char *text) { 

    if (connected_to == -1) {
        printf("Connect first\n");
        return;
    }
    send_msg(connected_to_queue, MESSAGE, text);
}

void parse_input(char *input) { 
    char *command;
    char *arg;
    command = strtok_r(input, " ", &arg);
    if (strncmp(command, "STOP", 4) == 0)
        exit(0);

    else if (strncmp(command, "DISCONNECT", 10) == 0)
        disconnect_action();

    else if (strncmp(command, "CONNECT", 7) == 0)
        connect_action(atoi(arg));

    else if (strncmp(command, "LIST", 4) == 0)
        list_action();

    else if (strncmp(command, "MSG", 3) == 0)
        msg_action(arg);

    else
        printf("Unknwon command: %s\n", command);

}


int main() { 

    atexit(stop_action);
    struct sigaction act;
    act.sa_handler = message_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGRTMIN, &act, NULL);

    signal(SIGINT, SIGINT_handler);
    srand(time(NULL));
    join_server();

    setup_notifications();

    char input[MAX_TEXT_SIZE];

    while (1) {
        if (fgets(input, sizeof(input), stdin) != NULL)
            parse_input(input);

    }
} 
