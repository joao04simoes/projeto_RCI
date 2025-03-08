#include "node.h"
#include "network.h"

void AddNodeFromNetList(Node *node, char *ip, int port)
{
    NodeList *newNode = (NodeList *)malloc(sizeof(NodeList));
    addInfoToNode(&newNode->data, ip, port, -1);
    newNode->next = node->netlist;
    node->netlist = newNode;
}

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
    while (curr)
    {
        SendSafeMsg(node->vzext.ip, node->vzext.port, curr->data.FD);
        curr = curr->next;
    }
}

void handleEntry(Node *node, int newfd, char *ip, int port)
{
    addInternalNeighbor(node, newfd, ip, port);
    if (node->vzext.FD != -1)
    {
        printf("enviar logo safe aaprit do ahndle entry fD:%d\n", node->vzext.FD);
        SendSafeMsg(node->vzext.ip, node->vzext.port, newfd);
    }
    else
    {
        printf("enviar logo sENTRY e safe aaprit do ahndle entry\n");
        addInfoToNode(&node->vzext, ip, port, newfd);
        SendEntryMsg(node->ip, node->port, newfd);
        updateInternalsSafe(node);
    }
}

void handleSafe(Node *node, char *ip, int port)
{
    addInfoToNode(&node->vzsalv, ip, port, -1);
}

void verifyExternal(Node *node) // esta funçao não funciona
{
    NodeList *curr = node->intr;
    removeInternalNeighbor(node, node->vzext.FD);
    close(node->vzext.FD);
    if (strcmp(node->ip, node->vzsalv.ip) == 0 && node->port == node->vzsalv.port)
    {
        // escolher um interno como externo
        curr = node->intr;
        while (curr)
        {
            if (curr->data.FD != -1)
            {
                printf("perdeu externo tem interno\n");
                addInfoToNode(&node->vzext, curr->data.ip, curr->data.port, curr->data.FD);
                SendEntryMsg(node->ip, node->port, node->vzext.FD);
                updateInternalsSafe(node);
                return;
            }
        }
        // não tem internos nem salvaguarda
        printf("perdeu externo não tem interno nem salvaguarda\n");
        addInfoToNode(&node->vzext, "", -1, -1);
        printf("escreve nada n o externos\n");
        return;
    }
    else
    {
        printf("perdeu externo tem salvaguarda\n");
        addInfoToNode(&node->vzext, node->vzsalv.ip, node->vzsalv.port, node->vzsalv.FD);
        printf("escreve o salva no externos\n");
        addInfoToNode(&node->vzsalv, "", -1, -1);
        printf("escreve nada n o salva\n");
        directJoin(node, node->vzext.ip, node->vzext.port);
        printf("mandou entry mesage\n");
        updateInternalsSafe(node);
        printf("update dso internal\n");
        return;
    }
}

void addInfoToNode(Info *info, char *ip, int port, int fd)
{
    info->FD = fd;
    info->port = port;
    strcpy(info->ip, ip);
}

void SendSafeMsg(char *ip, int port, int FD)
{
    char msg[128];
    sprintf(msg, "SAFE %s %d\n", ip, port);
    write(FD, msg, strlen(msg));
}
void SendEntryMsg(char *ip, int port, int FD)
{
    char msg[128];
    sprintf(msg, "ENTRY %s %d\n", ip, port);
    write(FD, msg, strlen(msg));
}
