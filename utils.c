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
    if (strcmp(cmd, "dj") == 0)

    {
        directJoin(node, arg1, arg2);
        return;
    }
    else if (strcmp(cmd, "st") == 0)
    {
        if (node->vzext.port != -1)
            printf("Vizinho externo: %s:%d\n", node->vzext.ip, node->vzext.port);
        NodeList *curr = node->intr;
        if (curr != NULL)
        {
            printf("Vizinhos internos:\n");
            while (curr)
            {
                printf("- %s:%d\n", curr->data.ip, curr->data.port);
                curr = curr->next;
            }
        }
        if (node->vzsalv.port != -1)
            printf("Salvagurada: %s:%d\n", node->vzsalv.ip, node->vzsalv.port);
    }
    else
    {
        printf("Comando desconhecido: %s\n", cmd);
    }
    return;
}
