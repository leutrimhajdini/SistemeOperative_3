
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define MSG_SIZE 256

typedef struct {
    int client_queue_id;
    pthread_t thread_id;
} Client;

typedef struct {
    int msg_type;
    char msg_text[MSG_SIZE];
} Message;

int server_queue_id;
Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;



void* clientThread(void* arg) {
    int client_queue_id = *((int*)arg);
    Message msg;

    while (1) {
        ssize_t recv_result = msgrcv(client_queue_id, &msg, sizeof(Message) - sizeof(long), 1, 0);
        if (recv_result == -1) {
            perror("msgrcv");
            fprintf(stderr, "Error receiving message from client %d, errno: %d\n", client_queue_id, errno);
            exit(1);
        } else if (recv_result == 0) {
            fprintf(stderr, "Client %d has disconnected\n", client_queue_id);
            break;
        }

        if (strncmp(msg.msg_text, "exit", 4) == 0) {
            fprintf(stdout, "Client %d has requested to exit\n", client_queue_id);
            break;
        }

        printf("Received message from client %d: %s\n", client_queue_id, msg.msg_text);


    }

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].client_queue_id == client_queue_id) {
            clients[i].client_queue_id = 0;
            clients[i].thread_id = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    pthread_exit(NULL);
}


void handleConnection(int client_queue_id) {
    int client_index = -1;
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].client_queue_id == 0) {
            client_index = i;
            clients[i].client_queue_id = client_queue_id;
            printf("Client %d connected.\n", client_queue_id);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    if (client_index == -1) {
        printf("Maximum client limit reached. Connection rejected.\n");
        return;
    }
    


    pthread_create(&clients[client_index].thread_id, NULL, clientThread, &clients[client_index].client_queue_id);
}

int main() {
    key_t key;
    Message msg;


    if ((key = ftok("Server.c", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((server_queue_id = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].client_queue_id = 0;
        clients[i].thread_id = 0;
    }

    printf("Server %d started. Listening for connections...\n", server_queue_id);
	while (1) {
	    if (msgrcv(server_queue_id, &msg, sizeof(Message) - sizeof(long), 1, 0) == -1) {
		perror("msgrcv");
		exit(1);
	    }

	    handleConnection(atoi(msg.msg_text));
		
	}

    return 0;
}
