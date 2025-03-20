#include "node.h"
#include "network.h"
#include "utils.h"
#include "cache.h"

int main(int argc, char *argv[])
{
    Node node;
    char defaultIP[16] = "193.136.138.142";
    char defaultPort[6] = "59000";

    if (argc > 4)
    {
        printf("mais de 4 argc\n");
        if (argv[4] != NULL)
        {
            printf("arg 4\n");
            strncpy(defaultIP, argv[4], sizeof(defaultIP) - 1);
        }
        if (argv[5] != NULL)
        {
            printf("arg 5\n");
            strncpy(defaultPort, argv[5], sizeof(defaultPort) - 1);
        }
    }

    // Se houver argumentos a mais, erro
    if (argc > 6)
    {
        fprintf(stderr, "Uso correto: %s cache IP TCP regIP regUDP\n", argv[0]);
        exit(1);
    }
    printf("copy to node\n");
    strncpy(node.regIP, defaultIP, sizeof(defaultIP) - 1);
    strncpy(node.regUDP, defaultPort, sizeof(defaultPort) - 1);

    int cache = atoi(argv[1]);
    char *tcpIP = argv[2];
    int tcpPort = atoi(argv[3]);

    char portStr[6];
    sprintf(portStr, "%d", tcpPort);

    struct addrinfo hints, *res;
    int errcode;
    struct sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    char command[128];
    NodeList *curr;

    int fd, newfd = -1, counter, maxfd;
    fd_set rfds;
    char buffer[128];

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
    node.vzext.ip[0] = '\0';
    node.vzsalv.ip[0] = '\0';
    node.Table = NULL;
    node.Objects = NULL;
    printf("init cache\n");
    initCache(&node, cache);
    printf("fez init\n");

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
        printf("while 1\n");
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(fd, &rfds);
        FD_SET(node.vzext.FD, &rfds);
        curr = node.intr;
        maxfd = fd;
        while (curr)
        {
            FD_SET(curr->data.FD, &rfds);
            if (curr->data.FD > maxfd)
                maxfd = curr->data.FD;
            curr = curr->next;
        }
        if (node.vzext.FD > maxfd)
            maxfd = node.vzext.FD;
        counter = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (counter == -1)
        {
            printf("erro no select\n");
            ExitNdn(&node);
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
                    printf("erro accepting socket\n");
                    ExitNdn(&node);
                }
                else
                {
                    if (newfd > maxfd)
                        maxfd = newfd;
                    excuteCommandFromBuffer(buffer, &node, newfd);
                    // handleEntry(&node, newfd, ip, port);
                }
            }
            if (FD_ISSET(node.vzext.FD, &rfds))
            {

                buffer[0] = 0;
                int er = read(node.vzext.FD, buffer, sizeof(buffer));
                if (er == 0)
                {
                    printf("o nó externo foi desligado\n");
                    verifyExternal(&node);
                }
                if (er == -1)
                {
                    printf("erro reading external\n");
                    ExitNdn(&node);
                }
                else
                {
                    printf("executar comando\n");
                    excuteCommandFromBuffer(buffer, &node, node.vzext.FD);
                }
            }

            curr = node.intr;
            while (curr)
            {
                if (FD_ISSET(curr->data.FD, &rfds) && curr->data.FD != node.vzext.FD)
                {
                    printf("nos internos\n");
                    buffer[0] = 0;
                    int er = read(curr->data.FD, buffer, sizeof(buffer));
                    if (er == 0)
                    {
                        printf("o nó interno foi desligado\n");
                        removeInternalNeighbor(&node, curr->data.FD);
                    }

                    if (er == -1)
                    {
                        printf("erro reading internal\n");
                        ExitNdn(&node);
                    }
                    else
                    {
                        printf("executar comando nos internos\n");
                        excuteCommandFromBuffer(buffer, &node, curr->data.FD);
                    }
                }
                curr = curr->next;
            }
        }
    }
}
