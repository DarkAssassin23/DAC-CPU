#pragma once

typedef struct Node
{
    void *next;
    int addr;
    char *function;
} Node;

Node *addNode(char *function, int addr, Node*head);
Node *insertNode(char *function, int addr, int pos, Node*head);
Node *deleteNode(char *function, int addr, Node*head);
int findNode(char *function, Node*head);
void printList(Node*head);