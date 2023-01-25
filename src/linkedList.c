#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linkedList.h"

Node *addNode(char *function, int addr, Node*head)
{
    Node *newNode = malloc(sizeof(Node));
    if (newNode == NULL)
        return head;

    newNode->function = function;
    newNode->addr = addr;

    if (head == NULL)
    {
        head = newNode;
        newNode->next = NULL;
    }
    else
    {
        newNode->next = head;
        head = newNode;
    }
    return head;
}

int findNode(char *function, Node*head)
{
    Node *current = head;
    while (current != NULL)
    {
        if (strcmp(current->function, function) == 0)
            return current->addr;

        current = current->next;
    }
    return -1;
}

void printList(Node*head)
{
    Node *current = head;
    while (current != NULL)
    {
        printf("Function name: %s, Located at address: 0x%x\n",
            current->function, current->addr);
        current = current->next;
    }
}