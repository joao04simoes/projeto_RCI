#include "node.h"
#include "network.h"
#include "utils.h"

// Envia mensagem de SAFE aos internos
void updateInternalsSafe(Node *node)
{
    NodeList *curr = node->intr;
    while (curr)
    {
        if (curr->data.FD != node->vzext.FD)
            SendSafeMsg(node->vzext.ip, node->vzext.port, curr->data.FD);
        curr = curr->next;
    }
}

// Realiza as operações necessárias para a entrada de um novo nó
void handleEntry(Node *node, int newfd, char *ip, int port)
{
    if (isInternal(node, ip, port) == 0)
        addInternalNeighbor(node, newfd, ip, port);
    if (node->vzext.FD != -1) // se tiver externo envia mensagem de safe
    {
        printf("enviar mensagemde safe %s:%d\n", node->vzext.ip, node->vzext.port);
        SendSafeMsg(node->vzext.ip, node->vzext.port, newfd);
    }
    else // como não tem externo manda mensagem de entry e safe
    {
        printf("enviar mensagem de entry %s:%d\n", node->ip, node->port);
        addInfoToNode(&node->vzext, ip, port, newfd);
        SendEntryMsg(node->ip, node->port, newfd);
        printf("enviar mensagemde safe %s:%d\n", node->vzext.ip, node->vzext.port);
        SendSafeMsg(node->vzext.ip, node->vzext.port, newfd);
        updateInternalsSafe(node);
    }
}

// Realiza as operações ao receber uma mensagem de SAFE
void handleSafe(Node *node, char *ip, int port)
{
    addInfoToNode(&node->vzsalv, ip, port, -1);
}

// Encontram um novo novo depois da saida do externo
void verifyExternal(Node *node)
{
    NodeList *curr = node->intr;
    removeInternalNeighbor(node, node->vzext.FD);
    close(node->vzext.FD);
    if (strcmp(node->ip, node->vzsalv.ip) == 0 && node->port == node->vzsalv.port) // se o salvaguarda é ele proprio
    {
        // escolher um interno como externo
        curr = node->intr;
        while (curr)
        {
            if (curr->data.FD != -1)
            {
                curr = randomNode(curr);
                printf("Perdeu externo mas tem internos\n");
                addInfoToNode(&node->vzext, curr->data.ip, curr->data.port, curr->data.FD);
                printf("enviar mensagem de entry %s:%d\n", node->ip, node->port);
                SendEntryMsg(node->ip, node->port, node->vzext.FD);
                printf("enviar mensagemde safe %s:%d\n", node->vzext.ip, node->vzext.port);
                SendSafeMsg(node->vzext.ip, node->vzext.port, node->vzext.FD);
                updateInternalsSafe(node);
                return;
            }
        }
        // não tem internos nem salvaguarda
        printf("perdeu externo e não tem salvaguarda nem internos\n");
        addInfoToNode(&node->vzext, "", -1, -1);
        addInfoToNode(&node->vzsalv, "", -1, -1);
        return;
    }
    else // fazer ligação ao no de salvaguarda
    {
        printf("Perdeu externo e tem salvaguarda\n");
        addInfoToNode(&node->vzext, node->vzsalv.ip, node->vzsalv.port, node->vzsalv.FD);
        addInfoToNode(&node->vzsalv, "", -1, -1);
        directJoin(node, node->vzext.ip, node->vzext.port);
        updateInternalsSafe(node);
        return;
    }
}
