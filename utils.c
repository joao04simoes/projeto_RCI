#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network.h"

// Checks if a given IP and port are already in the internal neighbors list
int isInternal(Node *node, char *ip, int port)
{
    NodeList *curr = node->intr;
    while (curr)
    {
        if (strcmp(curr->data.ip, ip) == 0 && curr->data.port == port)
            return 1; // Found in internal list
        curr = curr->next;
    }
    return 0; // Not found
}

// Prints an error message and exits
void errorExit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void executeCommand(char *command, Node *node)
{
    char cmd[16], arg1[16];
    int arg2;
    sscanf(command, "%s %s %d", cmd, arg1, &arg2);

    if (strcmp(cmd, "x") == 0)
    {
        ExitNdn(node);
    }

    if (strcmp(cmd, "j") == 0)
    {
        JoinNet(node, arg1);
        return;
    }
    if (strcmp(cmd, "dj") == 0)
    {
        printf("mandar directJoin\n");
        directJoin(node, arg1, arg2);
        return;
    }
    if (strcmp(cmd, "st") == 0)
    {

        printf("Vizinho externo: %s:%d\n", node->vzext.ip, node->vzext.port);
        NodeList *curr = node->intr;

        printf("Vizinhos internos:\n");
        while (curr)
        {
            printf("- %s:%d\n", curr->data.ip, curr->data.port);
            curr = curr->next;
        }

        printf("Salvagurada: %s:%d\n", node->vzsalv.ip, node->vzsalv.port);
        return;
    }

    printf("Comando desconhecido: %s\n", cmd);

    return;
}
void ExitNdn(Node *node)
{
    NodeList *curr = node->intr;
    while (curr)
    {
        close(curr->data.FD);
        curr = curr->next;
    }
    close(node->FD);
    exit(0);
}
