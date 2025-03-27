#include "network.h"
#include "utils.h"
#include "node.h"
#include <sys/time.h> // Para struct timeval

#define TIMEOUT_SEC 3 // Tempo limite de 3 segundos
#define MAX_RETRIES 3 // Número máximo de tentativas

void JoinNet(Node *node, char *Net)
{
    int fd, errcode, retries;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[400];
    char trash[128];
    NodeList *curr;
    struct timeval timeout;

    // Criar socket UDP
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        printf("Erro no socket client\n");
        ExitNdn(node);
    }

    // Definir timeout para recvfrom
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo(node->regIP, node->regUDP, &hints, &res);
    if (errcode != 0)
    {
        printf("Erro no getaddrinfo server\n");
        ExitNdn(node);
    }

    // Enviar pedido de lista de nós
    sprintf(buffer, "NODES %s\n", Net);
    printf("Enviando pedido de lista de nós\n");

    retries = 0;
    while (retries < MAX_RETRIES)
    {
        n = sendto(fd, buffer, 10, 0, res->ai_addr, res->ai_addrlen);
        if (n == -1)
        {
            printf("Erro no sendto server\n");
            ExitNdn(node);
        }

        addrlen = sizeof(addr);
        n = recvfrom(fd, buffer, 400, 0, (struct sockaddr *)&addr, &addrlen);
        if (n != -1)
        {
            printf("Recebido do servidor: %s\n", buffer);
            break; // Recebeu resposta, sai do loop
        }
        printf("Timeout, tentando novamente... (%d/%d)\n", retries + 1, MAX_RETRIES);
        retries++;
    }

    if (retries == MAX_RETRIES)
    {
        printf("Erro: Falha ao receber resposta do servidor\n");
        ExitNdn(node);
    }

    // Criar lista de nós e conectar-se a um
    MakeNetList(buffer, node);
    curr = node->netlist;
    if (curr != NULL)
    {
        curr = randomNode(curr);
        directJoin(node, curr->data.ip, curr->data.port);
    }

    // Registrar na rede
    sprintf(buffer, "REG %s %s %d\n", Net, node->ip, node->port);
    printf("Enviando pedido de registro na rede\n");
    retries = 0;
    while (retries < MAX_RETRIES)
    {
        n = sendto(fd, buffer, 128, 0, res->ai_addr, res->ai_addrlen);
        if (n == -1)
        {
            printf("Erro no sendto server\n");
            ExitNdn(node);
        }

        n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
        if (n != -1)
        {

            break; // Recebeu resposta, sai do loop
        }
        printf("Timeout no REG, tentando novamente... (%d/%d)\n", retries + 1, MAX_RETRIES);
        retries++;
    }

    if (retries == MAX_RETRIES)
    {
        printf("Erro: Falha no registro na rede\n");
        ExitNdn(node);
    }

    if (sscanf(buffer, "OKREG%s\n", trash) == 1)
    {
        printf("Registro feito com sucesso\n");
        node->NetReg = 1;
    }
    else
    {
        printf("Erro no registro\n");
    }

    freeaddrinfo(res);
    close(fd);
}

void directJoin(Node *node, char *connectIP, int connectTCP)
{
    struct addrinfo hints, *res;
    int errcode, JoinFD;

    char portStr[6];

    if (strcmp(connectIP, "0.0.0.0") == 0)
    {
        printf("ip 0.0.0.0 \n Rede criada\n");
        return;
    }

    if (strcmp(connectIP, node->ip) == 0 && connectTCP == node->port)
    {
        printf("No tentou ligar-se a si mesmo\n");
        return;
    }

    if (isInternal(node, connectIP, connectTCP) == 1)
    {
        printf("Os nos ja se encontram ligadosn\n");
        return;
    }
    printf("Ligação direta a %s:%d\n", connectIP, connectTCP);
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
    printf("enviar mensagem de entry %s:%d\n", node->ip, node->port);
    SendEntryMsg(node->ip, node->port, JoinFD);
    freeaddrinfo(res);

    return;
}

// Inicializa o socket de escuta
void initListenSochet(Node *node)
{
    struct addrinfo hints, *res;
    int errcode, fd;
    char portStr[6];
    sprintf(portStr, "%d", node->port);
    printf("Inicializando socket de escuta\n");
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        printf("erro a criar socket \n");
        ExitNdn(node);
    }

    // Permite reutilização da porta após fechar o socket
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        printf("erro a definir SO_REUSEADDR\n");
        ExitNdn(node);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((errcode = getaddrinfo(NULL, portStr, &hints, &res)) != 0)
    {
        printf("erro no getaddrinfo server \n");
        ExitNdn(node);
    }

    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1)
    {
        printf("erro a dar bind \n");
        ExitNdn(node);
    }

    if (listen(fd, 5) == -1)
    {
        printf("erro a dar listen \n");
        ExitNdn(node);
    }

    node->FD = fd;
    freeaddrinfo(res);
}