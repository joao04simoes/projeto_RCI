#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"
#include "utilsForObject.h"
#include "cache.h"

// Inicia procura por um objeto
void retrieveObject(Node *node, char *objectName)
{

    interestTable *objectEntry;
    char *object;

    object = findObjectInLIst(node, objectName);
    if (object != NULL) // verifica se tem o objeto na lista de objetos
    {
        printf("tem o objeto %s na lista de objetos\n", object);
        return;
    }
    object = findObjectInCache(node, objectName);
    if (object != NULL) // verifica se tem o objeto na cache.
    {
        printf("tem o objeto %s na cache\n", object);
        return;
    }
    objectEntry = findObjectInTable(node, objectName);
    if (objectEntry != NULL) // verifica se tem o objeto tem entrada na tabela de interesse
    {
        return;
    }
    objectEntry = NULL;
    if (node->intr != NULL || node->vzext.FD != -1) // envia mensagem de interrese se tiver interfaces
    {
        objectEntry = createEntryToInterestTable(node, objectName);
        sendInterestMessageToallInterface(node, objectName, objectEntry, -1);
        showInterestTable(node);
    }
}

// Trata de uma mensagem de interesse
void handleInterest(Node *node, int fd, char *objectName)
{
    interestTable *objectEntry;
    TableInfo *fdEntry;

    char *object;

    object = findObjectInLIst(node, objectName);
    if (object != NULL) // Se o nó tiver o objeto, então envia a correspondente mensagem de objeto por fd.
    {
        sendObjectMessage(fd, objectName);
        return;
    }
    object = findObjectInCache(node, objectName);
    if (object != NULL) // Se o nó tiver o objeto, então envia a correspondente mensagem de objeto por fd.
    {
        sendObjectMessage(fd, objectName);
        return;
    }

    objectEntry = findObjectInTable(node, objectName);
    if (objectEntry != NULL) // se não tiver o objeto mas houver entrada passa o fd para o estado de resposta
    {
        fdEntry = findFdEntryInEntries(objectEntry->entries, fd, 1);
        if (fdEntry != NULL) // se houver entrada da interface fd no estado de espera passar para o estado de resposta
        {
            fdEntry->state = 0;
            sendAbsenceIfNoInterest(objectEntry);
            showInterestTable(node);
            removeEntryFromInterestTable(node, objectName);
            showInterestTable(node);
        }
        else // criar entrada da interface se não houver
        {
            createEntryToObjectList(fd, 0, objectEntry);
            showInterestTable(node);
        }

        return;
    }

    if (objectEntry == NULL) // não há entrada do objeto na tabela de interresses
    {
        if (node->intr == NULL || (node->intr->next == NULL && node->intr->data.FD == node->vzext.FD)) // verificar se é a unica ligação se for enviar mensgaem de ausencia
        {

            sendAbsenceObjectMessage(fd, objectName);
            showInterestTable(node);
            removeEntryFromInterestTable(node, objectName);
            showInterestTable(node);
            return;
        }

        // enviar mensagens de interresse e criar entrada na tabela
        objectEntry = createEntryToInterestTable(node, objectName);
        createEntryToObjectList(fd, 0, objectEntry);
        sendInterestMessageToallInterface(node, objectName, objectEntry, fd);
        showInterestTable(node);
    }
}

// Trata de uma mensagem de objeto
void handleObjectMessage(Node *node, char *objectName)
{
    interestTable *objectEntry;
    TableInfo *fdEntry;
    objectEntry = findObjectInTable(node, objectName);
    if (objectEntry != NULL) // se houver entrada na tabela de interesse enviar mensagem de objeto para todos os fd em estado de resposta
    {
        fdEntry = objectEntry->entries;
        while (fdEntry)
        {
            if (fdEntry->state == 0) // enviar mensagem de objeto para todos os fd em estado de resposta
            {
                sendObjectMessage(fdEntry->fd, objectName);
            }
            fdEntry = fdEntry->next;
        }
        // apagar entrada na tabela
        removeEntryFromInterestTable(node, objectName);
        addToCache(node, objectName);
        showInterestTable(node);
        return;
    }
}

// Trata de uma mensagem de ausência de objeto
void handleAbsenceMessage(Node *node, int fd, char *objectName)
{
    int flagWAiting = 0;
    interestTable *objectEntry;
    TableInfo *fdEntry;
    objectEntry = findObjectInTable(node, objectName);
    if (objectEntry != NULL)
    {

        fdEntry = findFdEntryInEntries(objectEntry->entries, fd, 1);
        if (fdEntry) // se houver entrada na tabela de interesse com o fd no estado de espera passa para fechado
        {

            fdEntry->state = 2;
            flagWAiting = sendAbsenceIfNoInterest(objectEntry); // se não houver mais entradas em estado de espera envia mensagem de ausencia
            showInterestTable(node);
            if (flagWAiting == 1)
            { // se não houver mais entradas em estado de espera apaga a entrada
                removeEntryFromInterestTable(node, objectName);
                showInterestTable(node);
            }
        }
    }
}

// Envia mensagem de ausência de objeto se não houver interesse em espera
int sendAbsenceIfNoInterest(interestTable *objectEntry)
{
    TableInfo *curr = objectEntry->entries;
    int VerState = 0;
    while (curr) // verifica se há entradas em estado de espera
    {
        if (curr->state == 1)
        {
            VerState = 1;
        }
        curr = curr->next;
    }
    if (VerState == 0) // se não houver entradas em estado de espera envia mensagem de ausencia
    {
        curr = objectEntry->entries;
        while (curr)
        {
            if (curr->state == 0)
            {
                sendAbsenceObjectMessage(curr->fd, objectEntry->objectName);
            }
            curr = curr->next;
        }
        return 1;
    }
    return 0;
}
