#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <libgen.h>
#include <pthread.h>
#include "my_server_lib.h"

static unsigned int clientId = 0;

void *clientThread(void *data)
{
    CLIENT_THREAD_DATA *clientData = (CLIENT_THREAD_DATA*) data;
    char *client_input = (char*) malloc(sizeof(char)*4096);
    if (client_input == NULL) {
        puts("Error: Out of memory.");
        return 0;
    }
    char *clientName = (char*) malloc(sizeof(char)*40);
    if (clientName == NULL) {
        puts("Error: Out of memory.");
        return 0;
    }
    int retval = recv(clientData->clientSocket, clientName, 39, 0);
    if (retval <= 0)
        strcpy(clientName, "<unnamed>");
    clientName[retval] = '\0';
    NODE *curNode;
    CLIENT *clientPtr;
    int clientInputLength;

    while (1) {
        retval = recv(clientData->clientSocket, client_input, 4095, 0);
        if (retval <= 0) {
            printf("Connection lost with (%d) having name: %s.\n", clientData->clientId, clientName);
            break;
        }
        client_input[retval] = '\0';
        clientInputLength = retval;
        curNode = clientData->clientArray;
        for (; curNode != NULL; curNode = curNode->nextNode) {
            clientPtr = (CLIENT*) curNode->data;
            if (clientPtr->clientId == clientData->clientId)
				continue;
            retval = send(clientPtr->socket, client_input, clientInputLength, 0);
            if (retval == 0) {
                printf("Send(): Connection lost with client id: %d\n", clientPtr->clientId);
                return 0;
            } else if (retval == SOCKET_ERROR) return 0;
        }
    }
    closesocket(clientData->clientSocket);
    free(clientName);
    free(client_input);
    curNode = clientData->clientArray;
    unsigned int i = 0;
    for (; curNode != NULL; curNode = curNode->nextNode, i++) {
        clientPtr = (CLIENT*) curNode->data;
        if (clientPtr->clientId == clientData->clientId) break;
    }
    deleteNode(&clientData->clientArray, i);
    *(clientData->clientCount) -= 1;
    free(clientData);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s PORT IP_ADDRESSv4\n", basename(argv[0]));
        return 0;
    }
    if (!isNum(argv[1])) {
        printf("%s is not a number.\n", argv[1]);
        return 0;
    }
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        puts("Error: Cannot initialize winsock.");
        return 0;
    }
    SOCKET mainSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mainSocket == INVALID_SOCKET || mainSocket == SOCKET_ERROR) {
        puts("Error: Cannot create socket.");
        return 0;
    }

    SOCKADDR_IN serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(argv[1]));
    serverAddress.sin_addr.S_un.S_addr = inet_addr(argv[2]);

    if (bind(mainSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == INVALID_SOCKET) {
        puts("Error: Cannot set the server's address to the mentioned address.");
        return 0;
    }
    if (listen(mainSocket, SOMAXCONN) == SOCKET_ERROR) {
		puts("Error: Cannot fill the server queue with the clients.");
		return 0;
	}

    NODE *clientArray = NULL;
    SOCKADDR_IN *clientAddress;
    int sizeofClientAddress;
    SOCKET clientSocket;
    CLIENT_THREAD_DATA *clientThreadData;
    CLIENT *client;
    pthread_t clientThreadId;
    unsigned int clientCount = 0;

    puts("Waiting for clients...");
    while (1) {
        clientAddress = (SOCKADDR_IN*) malloc(sizeof(SOCKADDR_IN));
        if (clientAddress == NULL) {
            puts("Error: Out of memory.");
            return 0;
        }
        sizeofClientAddress = sizeof(SOCKADDR_IN);
        clientSocket = accept(mainSocket, (SOCKADDR*) clientAddress, &sizeofClientAddress);
        if (clientSocket == INVALID_SOCKET) break;

        client = (CLIENT*) malloc(sizeof(CLIENT));
        if (client == NULL) {
            puts("Error: Out of memory.");
            return 0;
        }

        client->address = clientAddress;
        client->clientId = clientId;
        client->socket = clientSocket;
        addNode(&clientArray, client); //Add the client in the list

        clientThreadData = (CLIENT_THREAD_DATA*) malloc(sizeof(CLIENT_THREAD_DATA));
        if (clientThreadData == NULL) {
            puts("Error: Out of memory.");
            return 0;
        }
        clientThreadData->clientAddress = clientAddress;
        clientThreadData->clientArray = clientArray;
        clientThreadData->clientSocket = clientSocket;
        clientThreadData->clientId = clientId;
        clientThreadData->clientCount = &clientCount;

        pthread_create(&clientThreadId, NULL, clientThread, clientThreadData);
        clientCount++;
        printf("Client count: %d\n", clientCount);
        clientId++;
    }

    closesocket(mainSocket);
    WSACleanup();
    return 1;
}
