#include "node.h"

void addInternalNeighbor(Node *node, int fd, char *ip, int port)
{
    NodeList *newNode = (NodeList *)malloc(sizeof(NodeList));
    newNode->data.FD = fd;
    newNode->data.port = port;
    strcpy(newNode->data.ip, ip);
    newNode->next = node->intr;
    node->intr = newNode;
}

void removeInternalNeighbor(Node *node, int fd)
{
    NodeList *curr = node->intr, *prev = NULL;
    while (curr)
    {
        if (curr->data.FD == fd)
        {
            if (prev)
                prev->next = curr->next;
            else
                node->intr = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void updateInternal(Node *node)
{
    NodeList *curr = node->intr;
    char msg[128];
    sprintf(msg, "SAFE %s %d\n", node->vzext.ip, node->vzext.port);
    while (curr)
    {
        send(curr->data.FD, msg, strlen(msg), 0);
        curr = curr->next;
    }
}

void handleEntry(Node *node, int newfd, char *ip, int port)
{
    addInternalNeighbor(node, newfd, ip, port);
    if (node->vzext.FD != -1)
    {
        char msg[128];
        sprintf(msg, "SAFE %s %d\n", node->vzext.ip, node->vzext.port);
        send(newfd, msg, strlen(msg), 0);
    }
    else
    {
        node->vzext.FD = newfd;
        node->vzext.port = port;
        strcpy(node->vzext.ip, ip);
        char msg[128];
        sprintf(msg, "ENTRY %s %d\n", node->ip, node->port);
        send(newfd, msg, strlen(msg), 0);
        updateInternal(node);
    }
}

void handleSafe(Node *node, char *ip, int port)
{
    if (node->vzsalv.FD == -1)
    {
        node->vzsalv.port = port;
        strcpy(node->vzsalv.ip, ip);
        node->vzsalv.FD = -1;
    }
}
