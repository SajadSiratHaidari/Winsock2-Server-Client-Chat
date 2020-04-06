#ifndef MY_SERVER_LIB_H__
#define MY_SERVER_LIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char isNum(char *str)
{
    int i = 0;
    for (; str[i] != '\0'; i++)
        if (!isdigit(str[i]))
            return 0;
    return 1;
}

typedef struct {
    SOCKET socket;
    SOCKADDR_IN *address;
    unsigned char clientId;
} CLIENT;

struct linked_list_node {
    void *data;
    struct linked_list_node *nextNode;
};
typedef struct linked_list_node NODE;

typedef struct {
    SOCKADDR_IN *clientAddress;
    SOCKET clientSocket;
    NODE *clientArray;
    unsigned int clientId;
    unsigned int *clientCount;
} CLIENT_THREAD_DATA;

char addNode(NODE **arrayList, void *data)
{
    NODE *prevNode = NULL, *curNode = *arrayList;
    NODE *newNode = (NODE *) malloc(sizeof(NODE));
    if (newNode == NULL) return 0;
    newNode->data = data;
    newNode->nextNode = NULL;

    while (curNode != NULL) {
        prevNode = curNode;
        curNode = curNode->nextNode;
    }
    if (prevNode == NULL) {
        *arrayList = newNode;
    } else {
        prevNode->nextNode = newNode;
    }
    return 1;
}

void freeLinkedList(NODE **arrayList)
{
    NODE *curNode = *arrayList, *prevNode = NULL;
    while (curNode != NULL) {
        prevNode = curNode;
        curNode = curNode->nextNode;
        free(prevNode);
    }
}

char deleteNode(NODE **arrayList, unsigned int index)
{
    NODE *prevNode = NULL, *curNode = *arrayList;
    unsigned int i = 0;
    while (i != index) {
        prevNode = curNode;
        curNode = curNode->nextNode;
        if (curNode == NULL) return 0; //Found nothing
        i++;
    }
    if (prevNode == NULL) {
        free(curNode);
        *arrayList = NULL;
    } else {
        prevNode->nextNode = curNode->nextNode;
        free(curNode);
    }
    return 1;
}

#endif // MY_SERVER_LIB_H__
