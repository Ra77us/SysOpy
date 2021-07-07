#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define MAX_MESSAGE_LENGTH 256
#define MAX_NAME_LENGTH 32

int server;
char buffer[MAX_MESSAGE_LENGTH + 1];
char client_name[MAX_NAME_LENGTH + 1];
char symbol;
char board[3][3] = {0};
char* command;
char* arg;
char* arg2;
int my_move = -1;
int oponent_move = -1;
int game_status = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t msg_cond = PTHREAD_COND_INITIALIZER;


void quit(){
	
    char message[MAX_MESSAGE_LENGTH + 1];
    sprintf(message, "QUIT$%s$", client_name);
    send(server, message, MAX_MESSAGE_LENGTH, 0);
    shutdown(server, SHUT_RDWR);
	close(server);
    exit(0);
}

int check_win(){
	char winner_symbol = 0;
	
	char s = board[0][0];
	if(s != 0){
		for(int i = 1; i < 3; i++){
			if(board[i][0] != s)
				s = 0;
		}
	}
	if(s != 0)
		winner_symbol = s;
		
	s = board[0][0];
	if(s != 0){
		for(int i = 1; i < 3; i++){
			if(board[0][i] != s)
				s = 0;
		}
	}
	if(s != 0)
		winner_symbol = s;
	
	if(board[0][0] != 0 && board[0][0] == board[1][1] && board[0][0] == board[2][2])
		winner_symbol = board[0][0];
		
	if(board[0][2] != 0 && board[0][2] == board[1][1] && board[0][2] == board[2][0]){
		winner_symbol = board[0][2];
	}
	
	if(winner_symbol == symbol){
		return 1;
	}
	if(winner_symbol == 0){
		int flag = 1;
		for(int i = 0; i < 3; i++)
			for(int j = 0; j < 3; j++)
				if(board[i][j] == 0)
					flag = 0;
		if(flag == 1){
			return 2;
		}
		return 0; //game pending
	}
	return 3;
}

void draw_board(){
	printf("=============\n");
	for(int i = 0; i < 3; i++){
		printf("| %d | %d | %d |\n", i*3 + 1, i*3 + 2, i*3 + 3);
		printf("|");
		for (int j = 0; j < 3; j++){
			char c = board[j][i];
			if (c == 0)
				c = ' ';
			printf(" %c |", c);
		}
		printf("\n");
		printf("|   |   |   |\n");
		printf("=============\n");
	}
}

int update_board(int opponent, int pos){
	if(pos < 0 || pos > 8 || board[pos%3][pos/3] != 0)
		return -1;
	if(opponent == 0){
		board[pos%3][pos/3] = symbol;
	}
	else{
		char s = (symbol == 'X') ? 'O' : 'X'; 
		board[pos%3][pos/3] = s;
	}

	game_status = check_win();

	return 1;
}

void connect_to_server(char *mode, char *server_name){

	 if (strcmp(mode, "local") == 0){
        server = socket(AF_UNIX, SOCK_STREAM, 0);

        struct sockaddr_un addr;
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, server_name);

        connect(server, (const struct sockaddr *)&addr, sizeof(struct sockaddr_un));
    }
    else if(strcmp(mode, "network") == 0){
        struct addrinfo *res;
        struct addrinfo hints;        
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        getaddrinfo("localhost", server_name, &hints, &res);

        server = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        connect(server, res->ai_addr, res->ai_addrlen);

        freeaddrinfo(res);
    }
    else{
    	fprintf(stderr,"Invalid connection mode\n");
    	exit(-1);
    }
    
    char message[MAX_MESSAGE_LENGTH + 1];
    sprintf(message, "CONNECT$%s$", client_name);
    send(server, message, MAX_MESSAGE_LENGTH, 0);
}

void unpack_msg(char* msg){
 	command = strtok(msg, "$");
    arg = strtok(NULL, "$");
    arg2 = strtok(NULL, "$");
}

void game(){
	printf("Game started\n");
	printf("You are playing as %c\n", symbol);
	printf("You are playing against %s\n", arg2);
	while(1){
	
		pthread_mutex_lock(&mutex);
		int flag = 0;
		while(my_move == 0){
			if(flag == 0){
				printf("Waiting for opponent's move\n");
				flag = 1;
			}
			pthread_cond_wait(&msg_cond, &mutex);
		}
		draw_board();
		pthread_mutex_unlock(&mutex);
		printf("Your move\n");
		int pos;
		do{     
        	scanf("%d", &pos);
        	pos--;
        } while (update_board(0, pos) == -1);
        char message[MAX_MESSAGE_LENGTH + 1];
    	sprintf(message, "MOVE$%s$%d", client_name, pos);
    	send(server, message, MAX_MESSAGE_LENGTH, 0);
        my_move = 0;
		draw_board();
		if(game_status != 0){
			if(game_status == 1)
				printf("Vicorty!\n");
			else if(game_status == 2)
				printf("Draw\n");
			else if(game_status == 3)
				printf("Defeat\n");
			quit();
		}
	}
}

void listener(){
	
	while(1){
	
		recv(server, buffer, MAX_MESSAGE_LENGTH, 0);
		unpack_msg(buffer);
        pthread_mutex_lock(&mutex);
        
        if (strcmp(command, "PING") == 0){	
        	char message[MAX_MESSAGE_LENGTH + 1];
            sprintf(message, "PING_BACK$%s$ ", client_name);
            send(server, message, MAX_MESSAGE_LENGTH, 0);
        }
        else if (strcmp(command, "QUIT") == 0){
        	printf("Disconnected\n");
            exit(0);
        }
        else if (strcmp(command, "CONNECT") == 0){
          	pthread_t game_thread;
          	symbol = arg[0];
          	my_move = (symbol == 'X') ? 1 : 0;
          	if(my_move == 0)
          		draw_board();
            pthread_create(&game_thread, NULL, (void *(*)(void *))game, NULL);
        }
        else if (strcmp(command, "NAME_TAKEN") == 0){
          	printf("Name already taken\n");
          	exit(0);
        }
        else if (strcmp(command, "WAITING") == 0){
        	printf("Waiting for opponent\n");
        }
        else if(strcmp(command, "OP_MOVED") == 0){
        	my_move = 1;
        	if(update_board(1, atoi(arg)) == -1){
        		fprintf(stderr, "Server error");
        		quit();
        	}
        	if(game_status != 0){
				if(game_status == 1)
					printf("Vicorty!\n");
				else if(game_status == 2){
					draw_board();
					printf("Draw\n");
				}
				else if(game_status == 3){
					draw_board();
					printf("Defeat\n");
				}
				quit();
			}
        }
	
		pthread_cond_signal(&msg_cond);
        pthread_mutex_unlock(&mutex);
	}
}

int main(int argc, char *argv[]){

	 if (argc != 4){
        fprintf(stderr, "Invalid number of arguments");
        return -1;
    }
    signal(SIGINT, quit);
    
    strcpy (client_name, argv[1]);
    connect_to_server(argv[2], argv[3]);
    listener();
}

