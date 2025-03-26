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

    errcode = getaddrinfo(node->regIP, node->regUDP, &hints, &res);
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

    MakeNetList(buffer, node);
    curr = node->netlist;
    if (curr != NULL)
    {
        curr = randomNode(curr);
        directJoin(node, curr->data.ip, curr->data.port);
    }

    sprintf(buffer, "REG %s %s %d\n", Net, node->ip, node->port);
    n = sendto(fd, buffer, 128, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1)
    {
        printf("erro no sendto server\n");
        ExitNdn(node);
    }

    buffer[0] = 0;
    n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);

    if (n == -1)
    {
        printf("erro no recvfrom\n");
        ExitNdn(node);
    }
    if (sscanf(buffer, "OKREG%s\n", trash) == 1)
    {
        printf("Registo feito com sucesso\n");
        node->NetReg = 1;
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
    int errcode, JoinFD;

    char portStr[6];

    if (strcmp(connectIP, "0.0.0.0") == 0)
    {
        printf("ip 0.0.0.0 criar a rede\n");

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

    if (connect(JoinFD, res->ai_addr, res->ai_addrlen) == -1)
    {
        printf("erro no connect\n");
        ExitNdn(node);
    }
    addInfoToNode(&node->vzext, connectIP, connectTCP, JoinFD);
    SendEntryMsg(node->ip, node->port, JoinFD);
    freeaddrinfo(res);

    return;
}