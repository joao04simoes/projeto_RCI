#include "network.h"
#include "utils.h"
#include "node.h"

void JoinNet(Node *node, char *Net)
{
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[400];
    char trash[128];
    NodeList *curr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo(SERVER, PORT, &hints, &res);
    if (errcode != 0)
    {
        printf("erro no getaddrinfo server \n");
        ExitNdn(node);
    }
    sprintf(buffer, "NODES %s\n", Net);
    printf("%s", buffer);
    n = sendto(fd, buffer, 10, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1)
    {
        printf("erro no sendto server\n");
        ExitNdn(node);
    }

    addrlen = sizeof(addr);

    // Receber resposta do servidor
    n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
    if (n == -1)
    {
        printf("erro no recvfrom\n");
        ExitNdn(node);
    }
    write(1, "Echo: ", 6);
    write(1, buffer, n);
    MakeNetList(buffer, node);
    // funçao para conetar a um no da lista
    curr = node->netlist->next->next->next;
    printf("List feita\n");
    if (curr == NULL)
    {
        printf("Lista vazia\n");
    }
    directJoin(node, curr->data.ip, curr->data.port);
    printf("Join feito\n");
    sprintf(buffer, "REG %s %s %d\n", Net, node->ip, node->port);
    n = sendto(fd, buffer, 128, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1)
    {
        printf("erro no sendto server\n");
        ExitNdn(node);
    }

    n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
    printf("recebido do server %s fim \n", buffer);
    if (n == -1)
    {
        printf("erro no recvfrom\n");
        ExitNdn(node);
    }
    if (sscanf(buffer, "OKREG%s\n", trash) == 1)
    {
        printf("Registo feito com sucesso\n");
    }
    else
    {
        printf("Erro no registo\n");
    }
    freeaddrinfo(res);
    close(fd);
    return;
}

void directJoin(Node *node, char *connectIP, int connectTCP)
{
    struct addrinfo hints, *res;
    int errcode, JoinFD, port, port2;
    char cmd[15], ip[20];
    char portStr[6];
    char cmd2[15], ip2[20];

    if (strcmp(connectIP, "0.0.0.0") == 0)
    {
        printf("ip 0.0.0.0");
        addInfoToNode(&node->vzsalv, node->ip, node->port, -1);
        return;
    }

    if (isInternal(node, connectIP, connectTCP) == 1)
    {
        printf("os nos ja se encontram ligadosn\n");
        return;
    }

    sprintf(portStr, "%d", connectTCP);
    if ((JoinFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("erro no socket\n");
        ExitNdn(node);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((errcode = getaddrinfo(connectIP, portStr, &hints, &res)) != 0)
    {
        printf("erro no getaddrinfo\n");
        ExitNdn(node);
    }
    printf("conectando %s : %d\n", connectIP, connectTCP);
    if (connect(JoinFD, res->ai_addr, res->ai_addrlen) == -1)
    {
        printf("erro no connect\n");
        ExitNdn(node);
    }

    node->vzext.port = connectTCP;
    strcpy(node->vzext.ip, connectIP);
    node->vzext.FD = JoinFD;

    char RecMsg[128];

    SendEntryMsg(node->ip, node->port, JoinFD);
    read(JoinFD, RecMsg, sizeof(RecMsg));
    if (sscanf(RecMsg, "%s %s %d\n", cmd, ip, &port) == 3 && strcmp(cmd, "SAFE") == 0)
    {
        printf("recebeu safe %s : %d \n", ip, port);
        handleSafe(node, ip, port);
        addInternalNeighbor(node, JoinFD, node->vzext.ip, node->vzext.port);
    }
    else
    {
        if (sscanf(RecMsg, "%s %s %d\n", cmd2, ip2, &port2) == 3 && strcmp(cmd2, "ENTRY") == 0)
        {
            printf("recebeu entry seguido de safe %s : %d \n", ip2, port2);
            handleEntry(node, JoinFD, ip, port);
            handleSafe(node, node->ip, node->port);
        }
    }
    freeaddrinfo(res);
    return;
}