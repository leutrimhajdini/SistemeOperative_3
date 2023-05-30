#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_SIZE 256

// Structure for message
typedef struct {
    int msg_type;
    char msg_text[MSG_SIZE];
} Message;

int server_queue_id;
int client_queue_id;

// Function to send a request to the server
void sendRequest(const char* request) {
    Message msg;

    // Set the message type as request
    msg.msg_type = 1;

    // Copy the request data into the message
    strncpy(msg.msg_text, request, MSG_SIZE);

    // Send the message to the server
    if (msgsnd(server_queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }else{
    printf("Message is sent successfully;\n");
    
    }
}

int main() {
    key_t key;
    Message msg;
    int number_of_requests;

    // Create a message queue for the client
    if ((key = ftok("Client.c", 'C')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((client_queue_id = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }

    // Connect to the server by sending a connection request
    if ((server_queue_id = msgget(ftok("Server.c", 'B'), 0)) == -1) {
        perror("msgget");
        printf("Client message queue ID: %d\n", client_queue_id);
        exit(1);
    }

    // Send connection request to the server
    sprintf(msg.msg_text, "%d", client_queue_id);
    msg.msg_type = 1;  // Connection request type
    if (msgsnd(server_queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    printf("Connected to the server %d.\n", server_queue_id);

    // Send requests to the server
    char request[MSG_SIZE];
    while (1) {
        printf("Enter a request (or 'exit' to quit): ");


	fgets(request, MSG_SIZE, stdin);
	request[strcspn(request, "\n")] = '\0';
        if (strcmp(request, "exit") == 0) {
            break;
        }

        sendRequest(request);

    }

    // Gracefully disconnect from the server
    msg.msg_type = 1;  // Disconnect request type
    if (msgsnd(server_queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    // Remove the client message queue
    if (msgctl(client_queue_id, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    printf("Disconnected from the server.\n");

    return 0;
}
