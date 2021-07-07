#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#define MAX_CLIENTS 5

int server_queue = -1;
client *connected_clients[MAX_CLIENTS] = {NULL};
int clients_num = 0;

void print_error(char *text, int with_errno) {
    if (with_errno == 1) {
        fprintf(stderr, "%s: %s \n", text, strerror(errno));
    } else {
        fprintf(stderr, "%s\n", text);
    }
    exit(-1);
}

void SIGINT_handler(int signo) {
    printf("SIGINT received. Shutting down server\n");
    exit(0);
}

void send_msg(int receiver_queue_id, msgbuf *message, pid_t receiver_pid) {
    if (msgsnd(receiver_queue_id, message, MSG_SIZE, 0) == -1) {
        print_error("Error while sending message from server\n", 1);
    }
    kill(receiver_pid, SIGRTMIN);
}

void get_msg(msgbuf *message) {
    if (msgrcv(server_queue, message, MSG_SIZE, -7, 0) == -1)
        print_error("Error while getting message from server queue", 1);
}

void init_action(msgbuf *message) {
    if (clients_num == MAX_CLIENTS) {
        msgbuf reply;
        reply.mtype = INIT;
        reply.sender_id = -1;
        send_msg(atoi(message->mtext), &reply, message->sender_pid);
        fprintf(stderr, "Cannot connect client - all slots taken\n");
        return;
    }

    int new_id = 0;
    while (connected_clients[new_id] != NULL) {
        new_id++;
    }


    client *new_client = calloc(1, sizeof(client));
    new_client->id = new_id;
    new_client->queue_id = atoi(message->mtext);
    new_client->connected_to = -1;
    new_client->pid = message->sender_pid;
    connected_clients[new_id] = new_client;

    msgbuf reply;
    reply.sender_id = new_id;
    reply.mtype = INIT;
    if (msgsnd(new_client->queue_id, &reply, MSG_SIZE, 0) == -1) {
        print_error("Error while sending message from server\n", 1);
    }

    clients_num++;
    printf("Client with id %d joined the server\n", new_id);

}

void disconnect_action(msgbuf *message) {
    client *client_a = connected_clients[message->sender_id];
    client *client_b = connected_clients[client_a->connected_to];


    msgbuf reply;
    reply.mtype = DISCONNECT;
    send_msg(client_b->queue_id, &reply, client_b->pid);

    client_a->connected_to = -1;
    client_b->connected_to = -1;
    printf("Disconnected clients %d and %d\n", client_a->id, client_b->id);
}

void stop_action(msgbuf *message) {
    client *to_del = connected_clients[message->sender_id];
    connected_clients[message->sender_id] = NULL;
    free(to_del);
    clients_num--;
    printf("Client %d left the server \n", message->sender_id);
}

void list_action(msgbuf *message) {
    int sender_id = message->sender_id;
    msgbuf reply;
    reply.mtype = LIST;
    char *text = calloc(MAX_TEXT_SIZE, sizeof(char));
    int text_pointer = 0;

    for (int i = 0; i < MAX_CLIENTS; i++) {

        if (i == sender_id) {
            sprintf(text + text_pointer, "Client %d: current client\n", i);
        } else if (connected_clients[i] != NULL && connected_clients[i]->connected_to == -1) {
            sprintf(text + text_pointer, "Client %d: available for connection\n", i);
        } else if (connected_clients[i] != NULL && connected_clients[i]->connected_to != -1) {
            sprintf(text + text_pointer, "Client %d: unavailable for connection\n", i);
        }
        text_pointer = strlen(text);
    }
    sprintf(reply.mtext, "%s", text);
    send_msg(connected_clients[sender_id]->queue_id, &reply, connected_clients[sender_id]->pid);
    free(text);
}

void connect_action(msgbuf *message) {
    int sender = message->sender_id;
    client *client_a = connected_clients[sender];
    int id = atoi(message->mtext);

    msgbuf reply;
    reply.mtype = CONNECT;

    if (id >= MAX_CLIENTS || connected_clients[id] == NULL || connected_clients[id]->connected_to != -1) {
        reply.sender_id = -1;
        send_msg(client_a->queue_id, &reply, client_a->pid);
        return;
    }

    client *client_b = connected_clients[id];
    client_a->connected_to = id;
    client_b->connected_to = sender;

    reply.sender_id = client_b->id;
    reply.sender_pid = client_b->pid;
    sprintf(reply.mtext, "%d", client_b->queue_id);
    send_msg(client_a->queue_id, &reply, client_a->pid);

    reply.sender_id = client_a->id;
    reply.sender_pid = client_a->pid;
    sprintf(reply.mtext, "%d", client_a->queue_id);
    send_msg(client_b->queue_id, &reply, client_b->pid);

    printf("Connected client %d and %d\n", client_a->id, client_b->id);
}

void kill_server() {
    msgbuf msg;
    msg.mtype = STOP;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connected_clients[i] != NULL)
            send_msg(connected_clients[i]->queue_id, &msg, connected_clients[i]->pid);
    }

    while (clients_num > 0) {
        if (msgrcv(server_queue, &msg, MSG_SIZE, -7, 0) == -1)
            print_error("Error while getting message from server queue during killing server", 1);
        if (msg.mtype == STOP)
            stop_action(&msg);
    }
    if (msgctl(server_queue, IPC_RMID, NULL) == -1) {
        print_error("Error while removing queue", 1);
    }
    printf("Server is down\n");
}

int main() {

    atexit(kill_server);
    signal(SIGINT, SIGINT_handler);

    char *home_path = getenv("HOME");
    if (home_path == NULL)
        print_error("Error while establishing home path", 1);

    key_t public_key = ftok(home_path, PROJ_ID);
    if (public_key == -1)
        print_error("Error while creating public key", 1);

    server_queue = msgget(public_key, IPC_CREAT | IPC_EXCL | 0666);
    if (server_queue == -1)
        print_error("Error while creating server queue", 1);
    printf("Server ready!\n");
    msgbuf message;

    while (1) {
        get_msg(&message);

        switch (message.mtype) {
            case STOP:
                stop_action(&message);
                break;
            case DISCONNECT:
                disconnect_action(&message);
                break;
            case LIST:
                list_action(&message);
                break;
            case CONNECT:
                connect_action(&message);
                break;
            case INIT:
                init_action(&message);
                break;
            default:
                print_error("Unknown message type in server queue", 0);
                break;
        }
    }
}
