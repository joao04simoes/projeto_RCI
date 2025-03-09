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
        strcpy(node->NET, arg1);
        JoinNet(node, arg1);
        return;
    }
    if (strcmp(cmd, "dj") == 0)
    {

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
    char buffer[128];
    int n;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    socklen_t addrlen;
    int errcode;

    NodeList *curr = node->intr, *next = NULL;
    while (curr)
    {
        close(curr->data.FD);
        curr = curr->next;
    }
    close(node->FD);
    if (node->NetReg == 1)
    {

        curr = node->netlist;
        while (curr)
        {
            next = curr->next;
            free(curr);
            curr = next;
        }
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd == -1)
        {
            perror("socket");
            exit(1);
        }
        errcode = getaddrinfo(SERVER, PORT, &hints, &res);
        if (errcode != 0)
        {
            printf("erro no getaddrinfo server \n");
            exit(1);
        }
        sprintf(buffer, "UNREG %s %s %d\n", node->NET, node->ip, node->port);
        n = sendto(fd, buffer, 10, 0, res->ai_addr, res->ai_addrlen);
        if (n == -1)
        {
            printf("erro no sendto server\n");
            exit(1);
        }

        n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
        if (n == -1)
        {
            printf("erro no recvfrom\n");
        }
        if (sscanf(buffer, "OKUNREG") == 1)
        {
            printf("Remoção do registo feita com sucesso\n");
        }
        else
        {
            printf("Erro na remoção\n");
        }
    }

    exit(0);
}

void MakeNetList(char *buffer, Node *node)
{
    char ipToJoin[20];
    int portToJoin;
    char *line = strtok(buffer, "\n"); // Divide o buffer por linhas
    if (line && strncmp(line, "NODESLIST", 9) == 0)
    {
        line = strtok(NULL, "\n"); // Pula a primeira linha (NODESLIST net)
    }

    while (line)
    {
        if (sscanf(line, "%s %d\n", ipToJoin, &portToJoin) == 2)
        {
            printf("Adicionando nó %s:%d à lista\n", ipToJoin, portToJoin);
            AddNodeFromNetList(node, ipToJoin, portToJoin);
            printf("%s:%d adicionado\n", node->netlist->data.ip, node->netlist->data.port);
        }
        line = strtok(NULL, "\n"); // Próxima linha
    }
}