#include "network.h"
#include "utils.h"
#include "node.h"

#define PORT "58000"
#define SERVER "tejo.tecnico.ulisboa.pt"

/*void JoinNet(Node *node, int Net)
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

    n = sendto(fd, "\n", 10, 0, res->ai_addr, res->ai_addrlen);
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

    write(1, "Echo: ", 6);
    write(1, buffer, n);

    freeaddrinfo(res);
    close(fd);
    return 0;
}*/

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

    if ((errcode = getaddrinfo(connectIP, portStr, &hints, &res)) != 0)
        exit(1);

    if (connect(JoinFD, res->ai_addr, res->ai_addrlen) == -1)
        exit(1);

    char sendMsg[128], RecMsg[128];

    sprintf(sendMsg, "ENTRY %s %d\n", node->ip, node->port);
    send(JoinFD, sendMsg, strlen(sendMsg), 0);
    recv(JoinFD, RecMsg, sizeof(RecMsg), 0);

    if (sscanf(RecMsg, "%s %s %d\n", cmd, ip, &port) == 3 && strcmp(cmd, "SAFE") == 0)
    {
        handleSafe(node, ip, port);
        addInternalNeighbor(node, JoinFD, node->vzext.ip, node->vzext.port);
    }
    else
    {

        if (sscanf(RecMsg, "%s %s %d\n %s %s %d\n", cmd, ip, &port, cmd2, ip2, &port2) == 6 && strcmp(cmd, "ENTRY") == 0)
        {
            handleEntry(node, JoinFD, ip, port);
            handleSafe(node, ip2, port2);
        }
        else
        {
            handleEntry(node, JoinFD, ip, port);
            if (sscanf(RecMsg, "%s %s %d\n", cmd, ip, &port) == 3 && strcmp(cmd, "SAFE") == 0)
            {
                handleSafe(node, ip, port);
            }
        }
    }
    return;
}