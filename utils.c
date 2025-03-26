#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network.h"
#include "object.h"
#include "utilsForObject.h"
#include "cache.h"

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

// executa o comando do utilizador
void executeCommand(char *command, Node *node)
{
    char cmd[16], arg1[16];
    int arg2;
    sscanf(command, "%s %s %d", cmd, arg1, &arg2);

    if (strcmp(cmd, "x") == 0) // exit
    {
        ExitNdn(node);
    }
    else if (strcmp(cmd, "c") == 0) // create
    {
        addObjectToList(node, arg1);
        return;
    }
    else if (strcmp(cmd, "sn") == 0) // show names
    {
        showNames(node);
        return;
    }
    else if (strcmp(cmd, "dl") == 0) // delete
    {
        deleteObject(node, arg1);
        return;
    }
    else if (strcmp(cmd, "r") == 0) // retrieve
    {
        retrieveObject(node, arg1);
        return;
    }
    else if (strcmp(cmd, "j") == 0) // join
    {
        if (node->FD == -1)
            initListenSochet(node);
        strcpy(node->NET, arg1);
        JoinNet(node, arg1);
        return;
    }
    else if (strcmp(cmd, "dj") == 0) // direct join
    {
        if (node->FD == -1)
            initListenSochet(node);
        directJoin(node, arg1, arg2);
        return;
    }
    else if (strcmp(cmd, "si") == 0) // show interest table
    {
        showInterestTable(node);
        return;
    }
    else if (strcmp(cmd, "l") == 0) // leave
    {
        leaveNet(node);
        return;
    }
    else if (strcmp(cmd, "st") == 0) // show topology
    {
        NodeList *curr = node->intr;
        printf("No: %s:%d\n", node->ip, node->port);
        printf("No externo: %s:%d\n", node->vzext.ip, node->vzext.port);
        printf("No interno:\n");
        while (curr)
        {
            printf("- %s:%d\n", curr->data.ip, curr->data.port);
            curr = curr->next;
        }
        printf("No de salvaguarda: %s:%d\n", node->vzsalv.ip, node->vzsalv.port);
        return;
    }
    else
    {
        printf("Comando desconhecido: %s\n", cmd);
        return;
    }
}

// desligar a aplicação
void ExitNdn(Node *node)
{
    leaveNet(node);
    memCleanup(node);

    exit(0);
}

// Cria a lista de nós da rede
void MakeNetList(char *buffer, Node *node)
{
    char ipToJoin[20];
    int portToJoin;
    char *saveptr;
    char *line = strtok_r(buffer, "\n", &saveptr);
    if (line && strncmp(line, "NODESLIST", 9) == 0)
    {
        line = strtok_r(NULL, "\n", &saveptr);
    }

    while (line)
    {
        if (sscanf(line, "%s %d\n", ipToJoin, &portToJoin) == 2)
        {
            AddNodeFromNetList(node, ipToJoin, portToJoin);
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }
}

// Executa um comando a partir de um buffer
void excuteCommandFromBuffer(char *buffer, Node *node, int fd)
{
    if (buffer == NULL)
    {
        fprintf(stderr, "Error: buffer is NULL\n");
        return;
    }

    char objectName[101] = {0};
    char ip[20] = {0};
    char cmd[20] = {0};
    int port = 0;
    char *saveptr;
    char *line = strtok_r(buffer, "\n", &saveptr);

    while (line)
    {

        printf("Comando recebido: %s\n", line);
        if (sscanf(line, "%s %s %d", cmd, ip, &port) == 3)
        {
            if (strcmp(cmd, "ENTRY") == 0)
                handleEntry(node, fd, ip, port);
            if (strcmp(cmd, "SAFE") == 0)
                handleSafe(node, ip, port);

            line = strtok_r(NULL, "\n", &saveptr);
            continue;
        }

        if (sscanf(line, "%s %s", cmd, objectName) == 2)
        {
            if (strcmp(cmd, "OBJECT") == 0)
                handleObjectMessage(node, objectName);
            if (strcmp(cmd, "INTEREST") == 0)
                handleInterest(node, fd, objectName);
            if (strcmp(cmd, "NOOBJECT") == 0)
                handleAbsenceMessage(node, fd, objectName);

            line = strtok_r(NULL, "\n", &saveptr);
            continue;
        }

        line = strtok_r(NULL, "\n", &saveptr);
    }
}

// Sai da rede
void leaveNet(Node *node)
{

    char buffer[128], trash[128];
    int n;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    socklen_t addrlen;
    int errcode;

    NodeList *curr = node->intr, *next = NULL;

    removeInternalNeighbor(node, node->vzext.FD);
    close(node->vzext.FD);
    addInfoToNode(&node->vzext, "", -1, -1);
    addInfoToNode(&node->vzsalv, "", -1, -1);
    while (curr) // remove os vizinhos internos
    {
        removeInternalNeighbor(node, curr->data.FD);
        close(curr->data.FD);
        curr = curr->next;
    }
    if (node->NetReg == 1) // desregistar-se da rede
    {
        node->NetReg = -1;
        curr = node->netlist;
        while (curr) // free da lista dos nos da rede
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
    close(node->FD);
    node->FD = -1;
}

// Mostra os objetos do nó
void showNames(Node *node)
{
    Names *curr = node->Objects;
    printf("Objetos do Nó:\n");
    while (curr)
    {
        printf("%s\n", curr->name);
        curr = curr->next;
    }
    printCache(node);
}

// Escolher um nó aleatório de uma lista de nos
NodeList *randomNode(NodeList *nodeList)
{
    NodeList *curr = nodeList;
    int counter = 0;
    printf("random node\n");
    while (curr)
    {
        counter++;
        curr = curr->next;
    }
    curr = nodeList;

    int random = rand() % counter;
    int i = 0;
    while (counter--)
    {
        printf("while counter\n");
        if (i == random)
        {
            printf("logo o primeiro\n");
            return curr;
        }
        curr = curr->next;
        i++;
    }
    return nodeList;
}

// Limpa a memória
void memCleanup(Node *node)
{
    // limpar a cache e lista de objetos
    Names *curr = node->Objects, *next = NULL;
    while (curr)
    {
        next = curr->next;
        free(curr);
        curr = next;
    }
    free(node->cache->items);
    free(node->cache);
    // free da tabela de interrese
    interestTable *currTable = node->Table, *nextTable = NULL;
    TableInfo *currInfo = NULL, *nextInfo = NULL;
    while (currTable)
    {
        nextTable = currTable->next;
        currInfo = currTable->entries;
        while (currInfo)
        {
            nextInfo = currInfo->next;
            free(currInfo);
            currInfo = nextInfo;
        }
        free(currTable);
        currTable = nextTable;
    }
}
// adiciona info sobre um certo nó
void addInfoToNode(Info *info, char *ip, int port, int fd)
{
    info->FD = fd;
    info->port = port;
    strcpy(info->ip, ip);
}
// Envia uma mensagem de SAFE
void SendSafeMsg(char *ip, int port, int FD)
{
    char msg[128];
    sprintf(msg, "SAFE %s %d\n", ip, port);
    write(FD, msg, strlen(msg));
}
// Envia uma mensagem de ENTRY
void SendEntryMsg(char *ip, int port, int FD)
{
    char msg[128];
    sprintf(msg, "ENTRY %s %d\n", ip, port);
    write(FD, msg, strlen(msg));
}

// Adiciona um nó à lista de nós da rede
void AddNodeFromNetList(Node *node, char *ip, int port)
{
    NodeList *newNode = (NodeList *)malloc(sizeof(NodeList));
    addInfoToNode(&newNode->data, ip, port, -1);
    newNode->next = node->netlist;
    node->netlist = newNode;
}

// Adciona um nó à lista de nós internos
void addInternalNeighbor(Node *node, int fd, char *ip, int port)
{
    NodeList *newNode = (NodeList *)malloc(sizeof(NodeList));
    addInfoToNode(&newNode->data, ip, port, fd);
    newNode->next = node->intr;
    node->intr = newNode;
}
// Remove um nó da lista de nós internos
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
