#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

int server_queue;
int client_queue;
int client_id;
pid_t connected_to_pid;
int connected_to = -1;
int connected_to_queue = -1;

void print_error(char *text, int with_errno) {
    if (with_errno == 1) {
        fprintf(stderr, "%s: %s \n", text, strerror(errno));
    } else {
        fprintf(stderr, "%s\n", text);
    }
    exit(-1);
}

void send_msg(int receiver_queue_id, msgbuf *message) {
    if (msgsnd(receiver_queue_id, message, MSG_SIZE, 0) == -1) {
        print_error("Error while sending message from client\n", 1);
    }
}

void send_msg_to_client(int receiver_queue_id, msgbuf *message, pid_t receiver_pid) {
    if (msgsnd(receiver_queue_id, message, MSG_SIZE, 0) == -1) {
        print_error("Error while sending message to client\n", 1);
    }
    kill(receiver_pid, SIGRTMIN);
}


void get_msg(int queue_id, msgbuf *message) {
    if (msgrcv(queue_id, message, MSG_SIZE, -7, 0) == -1)
        print_error("Error while receiving message in client \n", 1);
}

void SIGINT_handler(int signo) {
    printf("SIGINT received. Shutting down client\n");
    exit(0);
}

void message_handler(int signum) {
    msgbuf message;
    get_msg(client_queue, &message);

    switch (message.mtype) {
        case STOP:
            exit(0);
            break;
        case DISCONNECT:
            connected_to = -1;
            connected_to_queue = -1;
            printf("Disconnected\n");
            break;
        case CONNECT:
            if (message.sender_id == -1) {
                printf("Connection failed!\n");
                return;
            }
            connected_to = message.sender_id;
            connected_to_queue = atoi(message.mtext);
            connected_to_pid = message.sender_pid;
            printf("Connected with %d\n", connected_to);
            break;
        case LIST:
            printf("%s", message.mtext);
            break;
        case MESSAGE:
            printf("%d says: %s", message.sender_id, message.mtext);
            break;
        default:
            print_error("Unknown message type in server queue", 0);
            break;
    }
}

void disconnect_action() {
    if (connected_to == -1) {
        printf("Already disconnected \n");
        return;
    }
    msgbuf message;
    message.mtype = DISCONNECT;
    message.sender_id = client_id;
    send_msg(server_queue, &message);
    connected_to = -1;
    connected_to_queue = -1;
    printf("Disconnected\n");
}

void stop_action() {
    msgbuf message;
    message.mtype = STOP;
    message.sender_id = client_id;

    if (connected_to != -1)
        disconnect_action();

    send_msg(server_queue, &message);
    if (msgctl(client_queue, IPC_RMID, NULL) == -1) {
        print_error("Error while removing queue", 1);
    }
    printf("Stopped\n");
}

void join_server() {

    char *home_path = getenv("HOME");
    if (home_path == NULL)
        print_error("Error while establishing home path", 1);

    key_t public_key = ftok(home_path, PROJ_ID);
    if (public_key == -1)
        print_error("Error while creating public key", 1);

    server_queue = msgget(public_key, 0);
    if (server_queue == -1)
        print_error("Error while getting server queue id: %s\n", 1);

    key_t private_key = ftok(home_path, getpid());
    if (private_key == -1)
        print_error("Error while creating private key", 1);

    client_queue = msgget(private_key, IPC_CREAT | IPC_EXCL | 0666);
    if (client_queue == -1)
        print_error("Error while creating client queue", 1);


    msgbuf message;
    message.mtype = INIT;
    message.sender_pid = getpid();
    sprintf(message.mtext, "%d", client_queue);


    send_msg(server_queue, &message);
    get_msg(client_queue, &message);

    if (message.sender_id == -1) {
        printf("Cannot join server - server full!\n");
        exit(0);
    }
    client_id = message.sender_id;
    printf("Succesfully joined server. Client ID: %d\n", client_id);
}

void list_action() {
    msgbuf message;
    message.sender_id = client_id;
    message.mtype = LIST;
    send_msg(server_queue, &message);
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

    msgbuf message;
    message.sender_pid = getpid();
    message.sender_id = client_id;
    message.mtype = CONNECT;
    sprintf(message.mtext, "%d", id);
    send_msg(server_queue, &message);
    printf("Connecting to %d\n", id);
}


void msg_action(char *text) {

    if (connected_to == -1) {
        printf("Connect first\n");
        return;
    }
    msgbuf message;
    message.mtype = MESSAGE;
    message.sender_id = client_id;
    sprintf(message.mtext, "%s", text);
    send_msg_to_client(connected_to_queue, &message, connected_to_pid);
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

    join_server();

    char input[MAX_TEXT_SIZE];

    while (1) {
        if (fgets(input, sizeof(input), stdin) != NULL)
            parse_input(input);
    }

} 
