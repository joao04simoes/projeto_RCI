#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>

typedef struct Info
{
    int port;
    char ip[20];
    int FD;
} Info;

typedef struct NodeList
{
    Info data;
    struct NodeList *next;
} NodeList;

typedef struct
{
    Info vzext;     // Vizinho externo
    NodeList *intr; // Lista ligada de vizinhos internos
    Info vzsalv;    // Nó de salvaguarda
    int port;
    char ip[20];
} Node;

void addInternalNeighbor(Node *node, int fd, char *ip, int port)
{
    NodeList *newNode = (NodeList *)malloc(sizeof(NodeList));
    newNode->data.FD = fd;
    newNode->data.port = port;
    strcpy(newNode->data.ip, ip);
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

void directJoin(Node *node, char *connectIP, int connectTCP)
{
    struct addrinfo hints, *res;
    int errcode, JoinFD, port;
    char cmd[15], ip[20];

    char portStr[6];
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

    sprintf(sendMsg, "ENTRY %s %d", node->ip, node->port);
    send(JoinFD, sendMsg, strlen(sendMsg), 0);
    recv(JoinFD, RecMsg, sizeof(RecMsg), 0); // CORREÇÃO: usar recv em vez de recvfrom

    if (sscanf(RecMsg, "%s %s %d", cmd, ip, &port) == 3 && strcmp(cmd, "SAFE") == 0)
    {
        node->vzsalv.port = port;
        strcpy(node->vzsalv.ip, ip);
    }
    else
    {
        if (sscanf(RecMsg, "%s %s %d", cmd, ip, &port) == 3 && strcmp(cmd, "ENTRY") == 0)
        {
            addInternalNeighbor(node, JoinFD, ip, port);
        }
        recv(JoinFD, RecMsg, sizeof(RecMsg), 0);
        if (sscanf(RecMsg, "%s %s %d", cmd, ip, &port) == 3 && strcmp(cmd, "SAFE") == 0)
        {
            node->vzsalv.port = port;
            strcpy(node->vzsalv.ip, ip);
        }
    }
}

void executeCommand(char *command, Node *node)
{
    char cmd[16], arg1[16];
    int arg2;

    sscanf(command, "%s %s %d", cmd, arg1, &arg2);

    if (strcmp(cmd, "dj") == 0)
    {
        directJoin(node, arg1, arg2);
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
}

int main(int argc, char *argv[])
{

    if (argc != 6)
    {
        fprintf(stderr, "Uso correto: %s cache IP TCP regIP regUDP\n", argv[0]);
        exit(1);
    }

    int cache = atoi(argv[1]);
    char *tcpIP = argv[2];
    int tcpPort = atoi(argv[3]);
    char *regIP = argv[4];
    int regUDP = atoi(argv[5]);

    char portStr[6];
    sprintf(portStr, "%d", tcpPort);

    struct addrinfo hints, *res;
    int errcode;
    ssize_t n;
    struct sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    char command[128];
    NodeList *curr;

    char cmd[15], ip[20];
    int port;
    int fd, newfd, counter, maxfd;
    fd_set rfds;

    Node node;
    strcpy(node.ip, tcpIP);
    node.port = tcpPort;
    node.intr = NULL;
    node.vzext.FD = -1;
    node.vzsalv.FD = -1;
    node.vzext.port = -1;
    node.vzsalv.port = -1;
    node.vzext.port = -1;
    node.vzsalv.port = -1;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((errcode = getaddrinfo(NULL, portStr, &hints, &res)) != 0)
    {
        exit(1);
    }

    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1)
        exit(1);

    listen(fd, 5);
    maxfd = fd;
    while (1)
    {
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(fd, &rfds);
        curr = node.intr;
        while (curr)
        {
            FD_SET(curr->data.FD, &rfds);
            if (curr->data.FD > maxfd)
                maxfd = curr->data.FD;
            curr = curr->next;
        }

        counter = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (counter == -1)
        {
            perror("select");
            exit(1);
        }

        while (counter--)
        {
            if (FD_ISSET(0, &rfds))
            {
                fgets(command, sizeof(command), stdin);
                executeCommand(command, &node);
            }
            if (FD_ISSET(fd, &rfds))
            {
                char buffer[128];
                newfd = accept(fd, (struct sockaddr *)&addr, &addrlen);
                recv(newfd, buffer, sizeof(buffer), MSG_DONTWAIT);

                if (newfd == -1)
                {
                    perror("accept");
                }
                else
                {
                    FD_SET(newfd, &rfds);
                    if (newfd > maxfd)
                        maxfd = newfd;

                    if (sscanf(buffer, "%s %s %d", cmd, ip, &port) == 3 && strcmp(cmd, "ENTRY") == 0)
                    {
                        addInternalNeighbor(&node, newfd, ip, port);

                        if (node.vzext.FD != -1)
                        {
                            char msg[128];
                            sprintf(msg, "SAFE %s %d", node.vzext.ip, node.vzext.port);
                            send(newfd, msg, strlen(msg), 0);
                        }
                        else
                        {
                            node.vzext.FD = newfd;
                            node.vzext.port = port;
                            strcpy(node.vzext.ip, ip);

                            char msg[128];
                            sprintf(msg, "ENTRY %s %d", node.ip, node.port);
                            send(newfd, msg, strlen(msg), 0);

                            sprintf(msg, "SAFE %s %d", node.vzext.ip, node.vzext.port);
                            send(newfd, msg, strlen(msg), 0);
                        }
                    }
                }
            }
        }
    }
}
