#include "node.h"

void addInternalNeighbor(Node *node, int fd, char *ip, int port)
{
    NodeList *newNode = (NodeList *)malloc(sizeof(NodeList));
    addInfoToNode(&newNode->data, ip, port, fd);
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

void updateInternalsSafe(Node *node)
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
        write(newfd, msg, strlen(msg));
    }
    else
    {
        addInfoToNode(&node->vzext, ip, port, newfd);
        char msg[128];
        sprintf(msg, "ENTRY %s %d\n", node->ip, node->port);
        write(newfd, msg, strlen(msg));
        updateInternalsSafe(node);
    }
}

void handleSafe(Node *node, char *ip, int port)
{
    addInfoToNode(&node->vzsalv, ip, port, -1);
}

void verifyExternal(Node *node)
{
    char buffer[128];
    char msg[128];
    NodeList *curr = node->intr;
    if (read(node->vzext.FD, buffer, sizeof(buffer)) == 0)
    {
        removeInternalNeighbor(node, node->vzext.FD);
        close(node->vzext.FD);
        if (strcmp(node->ip, node->vzsalv.ip) == 0 && node->port == node->vzsalv.port)
        {
            // escolher um interno como externo
            while (curr)
            {
                if (curr->data.FD != -1)
                {
                    addInfoToNode(&node->vzext, curr->data.ip, curr->data.port, curr->data.FD);
                    sprintf(msg, "ENTRY %s %d\n", node->ip, node->port);
                    send(node->vzext.FD, msg, strlen(msg), 0);
                    updateInternalsSafe(node);
                    return;
                }
            }
            // não tem internos nem salvaguarda
            addInfoToNode(&node->vzext, '\0', -1, -1);
        }
        else
        {
            addInfoToNode(&node->vzext, node->vzsalv.ip, node->vzsalv.port, node->vzsalv.FD);
            addInfoToNode(&node->vzsalv, '\0', -1, -1);
            sprintf(msg, "ENTRY %s %d\n", node->ip, node->port);
            send(node->vzext.FD, msg, strlen(msg), 0);
            updateInternalsSafe(node);
        }
    }
}

void addInfoToNode(Info *info, char *ip, int port, int fd)
{
    info->FD = fd;
    info->port = port;
    strcpy(info->ip, ip);
}
// criar função de enviar entry e safe
