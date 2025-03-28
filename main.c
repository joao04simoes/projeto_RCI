#include "node.h"
#include "network.h"
#include "utils.h"
#include "cache.h"

int main(int argc, char *argv[])
{
    Node node;
    char defaultIP[16] = "193.136.138.142";
    char defaultPort[6] = "59000";
    struct sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    char command[128];
    NodeList *curr;
    int newfd = -1, counter, maxfd;
    fd_set rfds;
    char buffer[128];
    // verificação do ip do servidor UDP e da porta
    if (argc > 4 && argv[4] != NULL)
    {
        if (!isValidIP(argv[4]))
        {
            fprintf(stderr, "Erro: IP servidor UDP inválido.\n");
            exit(1);
        }
        strncpy(defaultIP, argv[4], sizeof(defaultIP) - 1);
    }
    if (argc > 5 && argv[5] != NULL)
    {
        if (!isValidPort(argv[5]))
        {
            fprintf(stderr, "Erro: Porta servidor UDP inválida.\n");
            exit(1);
        }
        strncpy(defaultPort, argv[5], sizeof(defaultPort) - 1);
    }

    // Se houver argumentos a mais, erro
    if (argc > 6)
    {
        fprintf(stderr, "Uso correto: %s cache IP TCP regIP regUDP\n", argv[0]);
        exit(1);
    }

    strncpy(node.regIP, defaultIP, sizeof(defaultIP) - 1);
    strncpy(node.regUDP, defaultPort, sizeof(defaultPort) - 1);
    // verificação dos argumentos

    if (!isValidPort(argv[1]))
    {
        fprintf(stderr, "Erro: O valor de cache deve ser um número válido.\n");
        exit(1);
    }
    if (!isValidIP(argv[2]))
    {
        fprintf(stderr, "Erro: IP do programa inválido.\n");
        exit(1);
    }

    if (!isValidPort(argv[3]))
    {
        fprintf(stderr, "Erro: Porta do programa inválida.\n");
        exit(1);
    }

    int cache = atoi(argv[1]);
    char *tcpIP = argv[2];
    int tcpPort = atoi(argv[3]);
    // Inicialização do nó
    strcpy(node.ip, tcpIP);
    node.FD = -1;
    node.NetReg = -1;
    node.port = tcpPort;
    node.intr = NULL;
    node.vzext.FD = -1;
    node.vzsalv.FD = -1;
    node.vzext.port = -1;
    node.vzsalv.port = -1;
    node.netlist = NULL;
    node.vzext.ip[0] = '\0';
    node.vzsalv.ip[0] = '\0';
    node.Table = NULL;
    node.Objects = NULL;
    initCache(&node, cache);
    // while para maquina de estados
    while (1)
    {

        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(node.FD, &rfds);
        FD_SET(node.vzext.FD, &rfds);
        curr = node.intr;
        if (node.FD > 0)
            maxfd = node.FD;
        else
            maxfd = 0;
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

        if (counter)
        {
            // verificar o stdin
            if (FD_ISSET(0, &rfds))
            {
                memset(command, 0, sizeof(command));
                command[127] = '\0';
                fgets(command, sizeof(command) - 1, stdin);
                executeCommand(command, &node);
            }
            // verificar socket de listen
            if (FD_ISSET(node.FD, &rfds))
            {
                memset(buffer, 0, sizeof(buffer));
                buffer[127] = '\0';
                newfd = accept(node.FD, (struct sockaddr *)&addr, &addrlen);
                recv(newfd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
                if (newfd == -1)
                {
                    printf("erro a aceitar coneção\n");
                    ExitNdn(&node);
                }
                else
                {
                    if (newfd > maxfd)
                        maxfd = newfd;
                    excuteCommandFromBuffer(buffer, &node, newfd);
                }
            }
            // verificar socket do externos
            if (FD_ISSET(node.vzext.FD, &rfds))
            {
                memset(buffer, 0, sizeof(buffer));
                buffer[127] = '\0';
                int er = read(node.vzext.FD, buffer, sizeof(buffer) - 1);
                if (er == 0)
                    verifyExternal(&node);

                if (er == -1)
                {
                    printf("erro a ler externos\n");
                    ExitNdn(&node);
                }
                if (er > 0)
                    excuteCommandFromBuffer(buffer, &node, node.vzext.FD);
            }

            curr = node.intr;
            // loop verificar os externos
            while (curr)
            {
                if (FD_ISSET(curr->data.FD, &rfds) && curr->data.FD != node.vzext.FD)
                {
                    memset(buffer, 0, sizeof(buffer));
                    buffer[127] = '\0';
                    int er = read(curr->data.FD, buffer, sizeof(buffer) - 1);
                    if (er == 0)
                        removeInternalNeighbor(&node, curr->data.FD);

                    if (er == -1)
                    {
                        printf("erro a ler internos\n");
                        ExitNdn(&node);
                    }
                    if (er > 0)
                        excuteCommandFromBuffer(buffer, &node, curr->data.FD);
                }
                curr = curr->next;
            }
        }
    }
}
