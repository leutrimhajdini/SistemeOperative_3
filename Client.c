#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_SIZE 256

typedef struct {
    int msg_type;
    char msg_text[MSG_SIZE];
} Message;

int server_queue_id;
int client_queue_id;

void sendRequest(const char* request) {
    Message msg;

    msg.msg_type = 1;

    strncpy(msg.msg_text, request, MSG_SIZE);

    if (msgsnd(server_queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }else{
    printf("Message is sent successfully;\n");
    }
}

void receiveResponse() {
    Message msg;

    if (msgrcv(client_queue_id, &msg, sizeof(Message) - sizeof(long), 2, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }

    printf("Received response from server: %s\n", msg.msg_text);
}

int main() {
    key_t key;
    Message msg;

    if ((key = ftok("Client.c", 'C')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((client_queue_id = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }

    if ((server_queue_id = msgget(ftok("Server.c", 'B'), 0)) == -1) {
        perror("msgget");
        printf("Client message queue ID: %d\n", client_queue_id);
        exit(1);
    }

    sprintf(msg.msg_text, "%d", client_queue_id);
    msg.msg_type = 1; 
    if (msgsnd(server_queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    printf("Connected to the server %d.\n", server_queue_id);


    char request[MSG_SIZE];
    while (1) {
        printf("Enter a request (or 'exit' to quit): ");


	fgets(request, MSG_SIZE, stdin);
	request[strcspn(request, "\n")] = '\0';
        if (strcmp(request, "exit") == 0) {
            break;
        }

        sendRequest(request);
        receiveResponse();
    }


    msg.msg_type = 1; 
    if (msgsnd(server_queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }


    if (msgctl(client_queue_id, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    printf("Disconnected from the server.\n");

    return 0;
}
