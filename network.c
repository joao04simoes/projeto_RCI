#include "network.h"
#include "utils.h"
#include "node.h"

#define PORT "59000"
#define SERVER "193.136.138.142"

void JoinNet(Node *node, char *Net)
{
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[128];

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
        perror("getaddrinfo");
        printf("error server");
        exit(1);
    }
    sprintf(buffer, "NODES %s\n", Net);
    printf("%s", buffer);
    n = sendto(fd, buffer, 10, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1)
    {
        perror("sendto");
        exit(1);
    }

    addrlen = sizeof(addr);

    // Receber resposta do servidor
    n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
    if (n == -1)
    {
        perror("recvfrom");
        exit(1);
    }
    printf("JoinNet\n");
    write(1, "Echo: ", 6);
    write(1, buffer, n);
    // if (sscanf(buffer, "NODESLIST %s\n%s %d\n",))

    // directJoin(node,)

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

    if (isInternal(node, connectIP, connectTCP) == 1)
    {
        printf("os nos ja se encontram ligadosn\n");
        return;
    }

    sprintf(portStr, "%d", connectTCP);
    if ((JoinFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exit(1);

    node->vzext.port = connectTCP;
    strcpy(node->vzext.ip, connectIP);
    node->vzext.FD = JoinFD;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    printf("directJoin\n");
    if ((errcode = getaddrinfo(connectIP, portStr, &hints, &res)) != 0)
        exit(1);
    printf("directJoin\n");
    if (connect(JoinFD, res->ai_addr, res->ai_addrlen) == -1)
        exit(1);
    printf("directJoin\n");
    char sendMsg[128], RecMsg[128];

    sprintf(sendMsg, "ENTRY %s %d\n", node->ip, node->port);
    write(JoinFD, sendMsg, strlen(sendMsg));
    read(JoinFD, RecMsg, sizeof(RecMsg));

    if (sscanf(RecMsg, "%s %s %d\n", cmd, ip, &port) == 3 && strcmp(cmd, "SAFE") == 0)
    {
        handleSafe(node, ip, port);
        addInternalNeighbor(node, JoinFD, node->vzext.ip, node->vzext.port);
    }
    else
    {
        printf("entry em vez de safe\n");
        if (sscanf(RecMsg, "%s %s %d\n", cmd2, ip2, &port2) == 3 && strcmp(cmd2, "ENTRY") == 0)
        {
            printf("nos dois\n");
            handleEntry(node, JoinFD, ip, port);
            printf("nos dois safe\n");
            handleSafe(node, node->ip, node->port);
        }
    }
    freeaddrinfo(res);
    return;
}