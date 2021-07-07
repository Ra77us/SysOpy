#define _POSIX_C_SOURCE 200112L

#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define MAX_CLIENTS 16
#define MAX_BACKLOG 16
#define MAX_MESSAGE_LENGTH 256
#define MAX_NAME_SIZE 32

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
    char name[MAX_NAME_SIZE];
    int fd;
    int online;
    int opponent;
    int in_use;
} client;

client clients[MAX_CLIENTS];

int clients_count = 0;
char path[40];
char port[40];
int local_socket;
int network_socket;

int get_client_id(char* name){
	for (int i = 0; i < MAX_CLIENTS; i++){
        if (clients[i].in_use != 0 && strcmp(clients[i].name, name) == 0){
            return i;
        }
    }
    return -1;
}

int add_client(int fd, char* name){
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (clients[i].in_use != 0 && strcmp(clients[i].name, name) == 0){
            return -1;
        }
    }
    int id = -1;
    int opponent_id = -1;
    int opponent_flag = 0;
    int id_flag = 0;
    
    for(int i = 0; i < MAX_CLIENTS; i++){
    	if (id_flag == 0 && clients[i].in_use == 0){
        	id = i;
            id_flag = 1;
    	}
    	if (opponent_flag == 0 && clients[i].in_use != 0 && clients[i].opponent == -1){
        	opponent_id = i;
            opponent_flag = 1;
    	}
    }
    
    if(id != -1){
        strcpy(clients[id].name, name);
        clients[id].fd = fd;
        clients[id].online = 1;
		clients[id].opponent = opponent_id;
		if(opponent_id != -1)
			clients[opponent_id].opponent = id;
        clients[id].in_use = 1;
        clients_count++;
    }
    
    return id;
}

void del_client(int id){
	printf("Deleting client: %s\n", clients[id].name);
	int opponent_id = clients[id].opponent;
    clients[id].in_use = 0;
    clients_count--;
   	if(id != -1 && clients[opponent_id].in_use != 0){
   		printf("Deleting client: %s\n", clients[opponent_id].name);
   		send(clients[opponent_id].fd, "QUIT$", MAX_MESSAGE_LENGTH, 0);
    	clients[opponent_id].in_use = 0;
    	clients_count--;
   	}
}

void check_client_status(){
    while (1){
        printf("Checking statuses and pinging...\n");
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < MAX_CLIENTS; i++){
		    if (clients[i].in_use != 0 && !clients[i].online){
		        del_client(i);
		    }
		    else if(clients[i].in_use != 0){
		     	clients[i].online = 0;
		        send(clients[i].fd, "PING$", MAX_MESSAGE_LENGTH, 0);
		    }
		}
		pthread_mutex_unlock(&mutex);
		sleep(5);
	}
}

void set_up_sockets(){
    local_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    bind(local_socket, (const struct sockaddr *)&addr, sizeof(struct sockaddr_un));
    listen(local_socket, MAX_BACKLOG);
       
    struct addrinfo *res;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, port, &hints, &res);
    network_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(network_socket, res->ai_addr, res->ai_addrlen);
    listen(network_socket, MAX_BACKLOG);
    freeaddrinfo(res);
}


int check_messages(int local_socket, int network_socket){
    struct pollfd pollfds[MAX_CLIENTS];
    pollfds[0].fd = local_socket;
    pollfds[0].events = POLLIN;
    pollfds[1].fd = network_socket;
    pollfds[1].events = POLLIN;
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < clients_count; i++){
        pollfds[i + 2].fd = clients[i].fd;
        pollfds[i + 2].events = POLLIN;
    }
    pthread_mutex_unlock(&mutex);
    poll(pollfds, clients_count + 2, -1);

    for (int i = 0; i < clients_count + 2; i++){
        if (pollfds[i].revents & POLLIN){
            int ret = pollfds[i].fd;
            if (ret == local_socket || ret == network_socket){
        		ret = accept(ret, NULL, NULL);
    		}
    		return ret;
        }
    }
    return -1;
}

void listner(){

	while(1){
		int client_fd = check_messages(local_socket, network_socket);
		char buffer[MAX_MESSAGE_LENGTH + 1];
		recv(client_fd, buffer, MAX_MESSAGE_LENGTH, 0);
		printf("Received message: %s\n", buffer);
		char *command = strtok(buffer, "$");
		char *name = strtok(NULL, "$");
		char *arg = strtok(NULL, "$");
		
		pthread_mutex_lock(&mutex);
		
		if (strcmp(command, "CONNECT") == 0){
            int id = add_client(client_fd, name);
            if (id == -1){
                send(client_fd, "NAME_TAKEN", MAX_MESSAGE_LENGTH, 0);
                close(client_fd);
            }
            else if (clients[id].opponent == -1){
                send(client_fd, "WAITING", MAX_MESSAGE_LENGTH, 0);
            }
            else{
                int first = rand() % 2;
                int opponent_id = clients[id].opponent;
                
                char message1[MAX_MESSAGE_LENGTH + 1];
                char message2[MAX_MESSAGE_LENGTH + 1];
                if(first == 0){
            		sprintf(message1, "CONNECT$X$%s$", clients[opponent_id].name);
            		sprintf(message2, "CONNECT$O$%s$", clients[id].name);
                }
                else{
                 	sprintf(message1, "CONNECT$O$%s$", clients[opponent_id].name);
            		sprintf(message2, "CONNECT$X$%s$", clients[id].name);
                }
                
		        send(clients[id].fd, message1 ,MAX_MESSAGE_LENGTH, 0);
		        send(clients[opponent_id].fd,message2, MAX_MESSAGE_LENGTH, 0);
            }
        }
        else if(strcmp(command, "MOVE") == 0){
            int pos = atoi(arg);
            int id = get_client_id(name);
			char message[MAX_MESSAGE_LENGTH + 1];
            sprintf(message, "OP_MOVED$%d", pos);
            send(clients[clients[id].opponent].fd, message, MAX_MESSAGE_LENGTH, 0);
        }
        else if(strcmp(command, "QUIT") == 0){
            del_client(get_client_id(name));
        }
        else if (strcmp(command, "PING_BACK") == 0){
            int id = get_client_id(name);
            clients[id].online = 1;
        }
		
		pthread_mutex_unlock(&mutex);
    }
}

void clean(){

	for(int i = 0; i< MAX_CLIENTS; i++){
		if(clients[i].in_use == 1)
			send(clients[i].fd, "QUIT$", MAX_MESSAGE_LENGTH, 0);
	}
	
	shutdown(local_socket, SHUT_RDWR);
	close(local_socket);
	unlink(path);
	
	shutdown(network_socket, SHUT_RDWR);
	close(network_socket);
}

void quit(){
	exit(0);
}

int main(int argc, char *argv[]){
    if (argc != 3){
    	fprintf(stderr, "Invalid number of arguments\n");
    	exit(-1);
    }
    
    strcpy(port, argv[1]);
    strcpy(path, argv[2]);
    for (int i = 0; i < MAX_CLIENTS; i++){
    	clients[i].in_use = 0;
    }
    atexit(clean);
    signal(SIGINT, quit);
    srand(time(NULL));
   	set_up_sockets();
    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, (void *(*)(void *))check_client_status, NULL);
    
    listner();
}


