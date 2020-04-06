#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <libgen.h>
#include <ctype.h>
#include <pthread.h>

char isNum(char *str)
{
    int i = 0;
    for (;str[i] != '\0'; i++)
        if (!isdigit(str[i])) return 0;
    return 1;
}

typedef struct {
    SOCKET clientSocket;
    pthread_t *otherThreadId;
    char *clientName;
} SEND_RECIEVE_THREAD_DATA;

void *recieveThread(void *data)
{
    SEND_RECIEVE_THREAD_DATA *threadData = (SEND_RECIEVE_THREAD_DATA*) data;
    char *otherPeopleMessage = (char*) malloc(sizeof(char)*4096);
    if (otherPeopleMessage == NULL) {
        puts("Error: Out of memory.");
        pthread_cancel(*(threadData->otherThreadId));
        return 0;
    }
    int retval;
    while (1) {
        retval = recv(threadData->clientSocket, otherPeopleMessage, 4095, 0);
        if (retval == 0)
            break;
        else if (retval == SOCKET_ERROR) {
            puts("Error: Cannot recieve any message from others in the group.");
            break;
        }
        otherPeopleMessage[retval] = '\0';
        puts(otherPeopleMessage);
    }
    free(otherPeopleMessage);
    pthread_cancel(*(threadData->otherThreadId));
    return 0;
}

void *sendThread(void *data)
{
    SEND_RECIEVE_THREAD_DATA *threadData = (SEND_RECIEVE_THREAD_DATA*) data;
    char *clientMessage = (char*) malloc(sizeof(char)*4096);
    if (clientMessage == NULL) {
        puts("Error: Out of memory.");
        pthread_cancel(*(threadData->otherThreadId));
        return 0;
    }
    int strLength, nameLength = strlen(threadData->clientName);
    char realName[42];
    strcpy(realName, threadData->clientName);
    realName[nameLength] = ':';
    realName[nameLength+1] = ' ';
    realName[nameLength+2] = '\0';
    nameLength += 2;

    puts("Enter your message (zero length means \"Exit\").\n");
    while (1) {
        printf("You: ");
        strcpy(clientMessage, realName);
        fgets(&clientMessage[nameLength], 4095-nameLength, stdin);
        strLength = strlen(&clientMessage[nameLength])+nameLength;
        if (clientMessage[strLength-1] == '\n') {
            clientMessage[strLength-1] = '\0';
            strLength--;
        }
        if (strLength == 0) break;
        if (send(threadData->clientSocket, clientMessage, strLength, 0) <= 0) {
            puts("Connection lost.");
            break;
        }
    }
    free(clientMessage);
    pthread_cancel(*(threadData->otherThreadId));
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s PORT IP_ADDRESS_V4", basename(argv[0]));
        return 0;
    }
    if (!isNum(argv[1])) {
        printf("%s is not a number.", argv[1]);
        return 0;
    }
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        puts("Error: Cannot initialize winsock.");
        return 0;
    }
    SOCKET mainSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mainSocket == INVALID_SOCKET) {
        puts("Error: Cannot create socket.");
        return 0;
    }
    SOCKADDR_IN clientAddress;
    clientAddress.sin_addr.S_un.S_addr = inet_addr(argv[2]);
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(atoi(argv[1]));
    if (connect(mainSocket, (SOCKADDR*) &clientAddress, sizeof(clientAddress)) == SOCKET_ERROR) {
        puts("Error: Cannot connect to the mentioned server.");
        return 0;
    }

    printf("Enter your name: ");
    char name[40];
    fgets(name, 40, stdin);
    int strLength = strlen(name);
    if (name[strLength-1] == '\n') {
        name[strLength-1] = '\0';
        strLength--;
    }
    int retval = send(mainSocket, name, strLength, 0);
    if (retval == SOCKET_ERROR) {
        puts("Error: Cannot send any data.");
        return 0;
    } else if (retval == 0) {
        puts("Connection lost!");
        return 0;
    }

    pthread_t recieveThreadId;
    pthread_t sendThreadId;
    SEND_RECIEVE_THREAD_DATA recieveData, sendData;

    recieveData.clientSocket = mainSocket;
    recieveData.otherThreadId = &sendThreadId;
    recieveData.clientName = name;
    sendData.clientSocket = mainSocket;
    sendData.otherThreadId = &recieveThreadId;
    sendData.clientName = name;

    pthread_create(&recieveThreadId, NULL, recieveThread, &recieveData);
    pthread_create(&sendThreadId, NULL, sendThread, &sendData);
    void *threadRetval;
    pthread_join(recieveThreadId, &threadRetval);
    pthread_join(sendThreadId, &threadRetval);
    closesocket(mainSocket);
    WSACleanup();
    return 0;
}
