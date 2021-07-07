#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <signal.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <sys/msg.h>

#define MAX_CLIENTS 5

mqd_t server_queue = -1;
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

void send_msg(mqd_t queue, unsigned int type, char *msg) {
    if (mq_send(queue, msg, strlen(msg), type) == -1) {
        print_error("Error while sending message from client\n", 1);
    }
}

void get_msg(mqd_t queue, unsigned int *type, char *msg) {
    if ((mq_receive(queue, msg, MAX_TEXT_SIZE, type)) == -1)
        print_error("Error while receiving message in server \n", 1);
}


void init_action(char *message) {
    mqd_t client_queue;
    if ((client_queue = mq_open(message, O_WRONLY)) == -1)
        print_error("Error while getting server queue", 1);

    if (clients_num == MAX_CLIENTS) {
        send_msg(client_queue, 6, "");
        fprintf(stderr, "Cannot connect client - all slots taken\n");
        mq_close(client_queue);
        return;
    }
    int new_id = 0;
    while (connected_clients[new_id] != NULL) {
        new_id++;
    }


    client *new_client = calloc(1, sizeof(client));
    new_client->id = new_id;
    new_client->queue = client_queue;
    new_client->connected_to = -1;
    sprintf(new_client->queue_name, "%s", message);
    connected_clients[new_id] = new_client;

    char text[10];
    sprintf(text, "%d", new_id);
    send_msg(client_queue, INIT, text);

    clients_num++;
    printf("Client with id %d joined the server\n", new_id);

}

void disconnect_action(char *message) {
    int sender_id = atoi(message);
    client *client_a = connected_clients[sender_id];
    client *client_b = connected_clients[client_a->connected_to];

    send_msg(client_b->queue, DISCONNECT, "");

    client_a->connected_to = -1;
    client_b->connected_to = -1;
    printf("Disconnected clients %d and %d\n", client_a->id, client_b->id);
}

void stop_action(char *message) {
    int sender_id = atoi(message);
    client *to_del = connected_clients[sender_id];
    mq_close(to_del->queue);
    connected_clients[sender_id] = NULL;
    free(to_del);
    clients_num--;
    printf("Client %d left the server \n", sender_id);
}

void list_action(char *message) {
    int sender_id = atoi(message);
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

    send_msg(connected_clients[sender_id]->queue, LIST, text);
    free(text);
}

void connect_action(char *message) {

    int id_a;
    int id_b;
    sscanf(message, "%d %d", &id_a, &id_b);
    client *client_a = connected_clients[id_a];

    if (id_b >= MAX_CLIENTS || connected_clients[id_b] == NULL || connected_clients[id_b]->connected_to != -1) {
        send_msg(client_a->queue, CONNECT, "-1:-1");
        return;
    }

    client *client_b = connected_clients[id_b];
    client_a->connected_to = id_b;
    client_b->connected_to = id_a;

    char text_a[50];
    sprintf(text_a, "%d %s", id_b, client_b->queue_name);
    send_msg(client_a->queue, CONNECT, text_a);

    char text_b[50];
    sprintf(text_b, "%d %s", id_a, client_a->queue_name);
    send_msg(client_b->queue, CONNECT, text_b);


    printf("Connected client %d and %d\n", client_a->id, client_b->id);
}

void kill_server() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connected_clients[i] != NULL)
            send_msg(connected_clients[i]->queue, STOP, "");
    }

    while (clients_num > 0) {
        char *text = calloc(10, sizeof(char));
        unsigned int type;
        get_msg(server_queue, &type, text);
        if (type == STOP)
            stop_action(text);
        free(text);
    }
    mq_close(server_queue);
    mq_unlink(SERVER_QUEUE_NAME);
    printf("Server is down\n");
}

int main() {

    atexit(kill_server);
    signal(SIGINT, SIGINT_handler);


    struct mq_attr attr;
    attr.mq_msgsize = MAX_TEXT_SIZE;
    attr.mq_maxmsg = MAX_MESSAGES;
    if ((server_queue = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr)) == -1)
        print_error("Error while creating queue", 1);

    printf("Server ready!\n");

    unsigned int type;
    while (1) {
        char *message = calloc(MAX_TEXT_SIZE + 1, sizeof(char));
        get_msg(server_queue, &type, message);

        switch (type) {
            case STOP:
                stop_action(message);
                break;
            case DISCONNECT:
                disconnect_action(message);
                break;
            case LIST:
                list_action(message);
                break;
            case CONNECT:
                connect_action(message);
                break;
            case INIT:
                init_action(message);
                break;
            default:
                print_error("Unknown message type in server queue", 0);
                break;
        }
        free(message);
    }

}
