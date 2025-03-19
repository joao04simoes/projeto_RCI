#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"
#include "utilsForObject.h"
#include "cache.h"

void addObjectToList(Node *node, char *objectName)
{
    Names *newObject = (Names *)malloc(sizeof(Names));
    strcpy(newObject->name, objectName);
    newObject->next = node->Objects;
    node->Objects = newObject;
}

char *findObjectInLIst(Node *node, char *objectName)
{
    Names *curr = node->Objects;
    while (curr)
    {
        if (strcmp(curr->name, objectName) == 0)
        {
            return curr->name;
        }
        curr = curr->next;
    }
    return NULL;
}

char *findObjectInCache(Node *node, char *objectName)
{
    int i;
    for (i = 0; i < node->cache->end; i++)
    {
        if (strcmp(node->cache->items[i].name, objectName) == 0)
        {
            return node->cache->items[i].name;
        }
    }
    return NULL;
}

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
    TableInfo *fdEntry, *curr;
    int VerState = 0;
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
        fdEntry = findFdEntryInEntries(objectEntry->entries, fd);
        if (fdEntry != NULL)
        { // if fd in entry list in state 1 (await) put state as 0 reponse else create the entry
            fdEntry->state = 0;
        }
        else
        {
            createEntryToObjectList(fd, 0, objectEntry);
            curr = objectEntry->entries;
            while (curr)
            {
                if (curr->state == 1)
                {
                    VerState = 1;
                    // verify that if there is a entry in state 1
                }
                curr = curr->next;
            }
            if (VerState == 0)
            {
                sendAbsenceObjectMessage(fd, objectName);
            }
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
        sendInterestMessageToallInterface(node, objectName, objectEntry, fd);
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
        printf("%s\n", node->cache->items[0].name);
        return;
    }
}

void handleAbsenceMessage(Node *node, int fd, char *objectName)
{
    interestTable *objectEntry;
    TableInfo *fdEntry;
    // mudar o estado para fechado talvez
    objectEntry = findObjectInTable(node, objectName);
    if (objectEntry != NULL)
    {
        fdEntry = findFdEntryInEntries(objectEntry->entries, fd);
        fdEntry->state = 2; // estado fechado
    }
}
