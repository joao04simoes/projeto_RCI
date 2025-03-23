#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"
#include "utilsForObject.h"
#include "cache.h"

void retrieveObject(Node *node, char *objectName)
{

    interestTable *objectEntry;
    char *object;

    object = findObjectInLIst(node, objectName);
    if (object != NULL) // Se o nó tiver o objeto, então envia a correspondente mensagem de objeto por N.
    {
        printf("tem o objeto %s\n", object);
        return;
    }
    object = findObjectInCache(node, objectName);
    if (object != NULL) // Se o nó tiver o objeto, então envia a correspondente mensagem de objeto por N.
    {
        printf("tem o objeto %s\n", object);
        return;
    }

    printf("retrieve\n");
    if (node->intr != NULL || node->vzext.FD != -1)
    {
        printf("dentro do if retrieve\n");
        objectEntry = createEntryToInterestTable(node, objectName);
        sendInterestMessageToallInterface(node, objectName, objectEntry, -1);
    }
}

void handleInterest(Node *node, int fd, char *objectName)
{
    interestTable *objectEntry;
    TableInfo *fdEntry;

    char *object;

    object = findObjectInLIst(node, objectName);
    if (object != NULL) // Se o nó tiver o objeto, então envia a correspondente mensagem de objeto por N.
    {
        sendObjectMessage(fd, objectName);
        return;
    }
    object = findObjectInCache(node, objectName);
    if (object != NULL) // Se o nó tiver o objeto, então envia a correspondente mensagem de objeto por N.
    {
        sendObjectMessage(fd, objectName);
        return;
    }

    objectEntry = findObjectInTable(node, objectName); // se não tiver mas houver entrada passar fd para o esatdo de resposta
    if (objectEntry != NULL)
    {
        fdEntry = findFdEntryInEntries(objectEntry->entries, fd, 1);
        if (fdEntry != NULL)
        { // if fd in entry list in state 1 (await) put state as 0 reponse else create the entry
            fdEntry->state = 0;
            sendAbsenceIfNoInterest(objectEntry);
        }
        else
        {
            createEntryToObjectList(fd, 0, objectEntry);
        }

        return;
    }

    if (objectEntry == NULL)
    {
        if (node->intr == NULL || (node->intr->next == NULL && node->intr->data.FD == fd)) // verificar se é a unica ligação se for enviar mensgaem de ausencia
        {
            sendAbsenceObjectMessage(fd, objectName);
            return;
        }

        // else enviar mensagens de objeto e criar entrada na tabela
        printf("enviar mesnagem de intereste\n");
        objectEntry = createEntryToInterestTable(node, objectName);
        createEntryToObjectList(fd, 0, objectEntry);
        sendInterestMessageToallInterface(node, objectName, objectEntry, fd); // não enviar para o fd que enviou a mensagem
    }
}

void handleObjectMessage(Node *node, char *objectName)
{
    interestTable *objectEntry;
    TableInfo *fdEntry;
    printf("handle object\n");
    objectEntry = findObjectInTable(node, objectName);
    printf("object entry\n");
    if (objectEntry != NULL)
    {
        printf("object entry\n");
        fdEntry = objectEntry->entries;
        while (fdEntry)
        {
            if (fdEntry->state == 0)
            {
                printf("enviar o objeto \n");
                sendObjectMessage(fdEntry->fd, objectName);
            }
            fdEntry = fdEntry->next;
        }
        printf("adiconar objeto a cache\n");
        // apagar entrada na tabela e
        removeEntryFromInterestTable(node, objectName);
        printf("adiconar objeto a cache\n");
        addToCache(node, objectName);

        return;
    }
}

void handleAbsenceMessage(Node *node, int fd, char *objectName)
{
    interestTable *objectEntry;
    TableInfo *fdEntry;

    objectEntry = findObjectInTable(node, objectName);
    if (objectEntry != NULL)
    {
        fdEntry = findFdEntryInEntries(objectEntry->entries, fd, 1);
        if (fdEntry)
        {
            fdEntry->state = 2; // Estado fechado
            sendAbsenceIfNoInterest(objectEntry);
        }
    }
}

void sendAbsenceIfNoInterest(interestTable *objectEntry)
{
    TableInfo *curr = objectEntry->entries;
    int VerState = 0;
    while (curr)
    {
        if (curr->state == 1)
        {
            VerState = 1;
            // verify that if there is a entry in state 1 (wait)
        }
        curr = curr->next;
    }
    if (VerState == 0)
    {
        curr = objectEntry->entries;
        while (curr)
        {
            if (curr->state == 0)
            {
                curr->state = 2; // perguntar se é para apagar da tabela
                sendAbsenceObjectMessage(curr->fd, objectEntry->objectName);
            }
            curr = curr->next;
        }
    }
}
