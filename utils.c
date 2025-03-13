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
    if (strcmp(cmd, "l") == 0)
    {
        leaveNet(node);
        return;
    }
    if (strcmp(cmd, "st") == 0)
    {
        NodeList *curr = node->intr;
        printf("Nó: %s:%d\n", node->ip, node->port);
        printf("Vizinho externo: %s:%d\n", node->vzext.ip, node->vzext.port);

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

    leaveNet(node);

    close(node->FD);
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

void excuteCommandFromBuffer(char *buffer, Node *node, int fd)
{
    char ip[20];
    int port;
    char cmd[20];
    char *line = strtok(buffer, "\n"); // Divide o buffer por linhas
    if (line && strncmp(line, "NODESLIST", 9) == 0)
    {
        line = strtok(NULL, "\n"); // Pula a primeira linha (NODESLIST net)
    }

    while (line)
    {
        if (sscanf(line, "%s %s %d\n", cmd, ip, &port) == 3)
        {
            if (strcmp(cmd, "ENTRY") == 0)
                handleEntry(node, fd, ip, port);
            if (strcmp(cmd, "SAFE") == 0)
                handleSafe(node, ip, port);
        }
        line = strtok(NULL, "\n"); // Próxima linha
    }
}

void leaveNet(Node *node)
{

    char buffer[128], trash[128];
    int n;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    socklen_t addrlen;
    int errcode;

    NodeList *curr = node->intr, *next = NULL;

    // remover externo porque pode não ser interno
    close(node->vzext.FD);
    removeInternalNeighbor(node, node->vzext.FD);
    addInfoToNode(&node->vzext, "", -1, -1);
    addInfoToNode(&node->vzsalv, node->ip, node->port, -1);
    while (curr)
    {
        removeInternalNeighbor(node, curr->data.FD);
        close(curr->data.FD);
        curr = curr->next;
    }
    if (node->NetReg == 1)
    {
        node->NetReg = -1;
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
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        errcode = getaddrinfo(node->regIP, node->regUDP, &hints, &res);
        if (errcode != 0)
        {
            printf("erro no getaddrinfo server \n");
            exit(1);
        }
        sprintf(buffer, "UNREG %s %s %d\n", node->NET, node->ip, node->port);
        n = sendto(fd, buffer, 128, 0, res->ai_addr, res->ai_addrlen);
        if (n == -1)
        {
            printf("erro no sendto server\n");
            exit(1);
        }
        buffer[0] = 0;
        n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
        if (n == -1)
        {
            printf("erro no recvfrom\n");
        }

        printf("recebido do server %s fim \n", buffer);
        if (sscanf(buffer, "OKUNREG%s", trash) == 1)
        {
            printf("Remoção do registo feita com sucesso\n");
        }
        else
        {
            printf("Erro na remoção\n");
        }
    }
}