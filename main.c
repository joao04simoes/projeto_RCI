#include "node.h"
#include "network.h"
#include "utils.h"

int main(int argc, char *argv[])
{

    if (argc != 6)
    {
        fprintf(stderr, "Uso correto: %s cache IP TCP regIP regUDP\n", argv[0]);
        exit(1);
    }

    // int cache = atoi(argv[1]);
    char *tcpIP = argv[2];
    int tcpPort = atoi(argv[3]);
    // char *regIP = argv[4];
    // int regUDP = atoi(argv[5]);

    char portStr[6];
    sprintf(portStr, "%d", tcpPort);

    struct addrinfo hints, *res;
    int errcode;
    struct sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    char command[128];
    NodeList *curr;

    char cmd[15], ip[20];
    int port;
    int fd, newfd = -1, counter, maxfd;
    fd_set rfds;
    char buffer[128];

    Node node;
    strcpy(node.ip, tcpIP);
    node.FD = -1;
    node.NetReg = -1;
    node.port = tcpPort;
    node.intr = NULL;
    node.vzext.FD = -1;
    node.vzsalv.FD = -1;
    node.vzext.port = -1;
    node.vzsalv.port = -1;
    node.vzext.port = -1;
    node.vzsalv.port = -1;
    node.netlist = NULL;

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
    if (listen(fd, 5) == -1)
        exit(1);

    maxfd = fd;
    node.FD = fd;
    freeaddrinfo(res);
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
                newfd = accept(fd, (struct sockaddr *)&addr, &addrlen);
                recv(newfd, buffer, sizeof(buffer), MSG_DONTWAIT);
                if (newfd == -1)
                {
                    perror("accept");
                }
                else
                {
                    if (newfd > maxfd)
                        maxfd = newfd;
                    if (sscanf(buffer, "%s %s %d\n", cmd, ip, &port) == 3 && strcmp(cmd, "ENTRY") == 0)
                    {
                        printf("recebeu direct join %s : %d \n", ip, port);
                        handleEntry(&node, newfd, ip, port);
                    }
                }
            }

            curr = node.intr;
            while (curr)
            {
                if (FD_ISSET(curr->data.FD, &rfds))
                {
                    buffer[0] = 0;
                    int er = read(curr->data.FD, buffer, sizeof(buffer));
                    if (er == 0 && strcmp(curr->data.ip, node.vzext.ip) == 0 && curr->data.port == node.vzext.port)
                    {
                        printf("o no externo foi desligado\n");
                        verifyExternal(&node);
                    }
                    else
                    {
                        if (er == 0)
                        {
                            printf("o no interno foi desligado\n");
                            removeInternalNeighbor(&node, curr->data.FD);
                        }
                    }

                    if (er == -1)
                    {
                        perror("read intr");
                    }
                    else
                    {

                        if (sscanf(buffer, "%s %s %d\n", cmd, ip, &port) == 3 && strcmp(cmd, "ENTRY") == 0)
                        {
                            printf("recebeu entry seguido de safe %s : %d \n", ip, port);
                            handleEntry(&node, curr->data.FD, ip, port);
                            handleSafe(&node, node.ip, node.port);
                        }
                        if (sscanf(buffer, "%s %s %d\n", cmd, ip, &port) == 3 && strcmp(cmd, "SAFE") == 0)
                        {
                            printf("recebeu safe %s : %d \n", ip, port);
                            handleSafe(&node, ip, port);
                        }
                    }
                }
                curr = curr->next;
            }
        }
    }
}
