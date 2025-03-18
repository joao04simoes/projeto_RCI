#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"
#include "utilsForObject.h"

void retrieveObject(Node *node, char *objectName)
{
    // verificar se tenho o obejeto

    TableInfo *objectEntry;

    objectEntry = createEntryToInterestTable(node, objectName);
    sendInterestMessageToallInterface(node, objectName, objectEntry, -1);
}

void handleInterest(Node *node, int fd, char *objectName)
{
    interestTable *objectEntry, *objectEntry;
    TableInfo *fdEntry, *curr;
    int VerState = 0;

    // Se o nó tiver o objeto, então envia a correspondente mensagem de objeto por N.

    objectEntry = findObjectInTable(node, objectName); // se não tiver mas houver entrada passar fd para o esatdo de resposta
    if (objectEntry != NULL)
    {
        fdEntry = findFdEntryInEntries(objectEntry, fd);
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
        objectEntry = createEntryToInterestTable(node, objectName);
        sendInterestMessageToallInterface(node, objectName, objectEntry, fd);
    }
}

void handleObjectMessage(Node *node, int fd, char *objectName)
{
    interestTable *objectEntry;
    TableInfo *fdEntry;
    objectEntry = findObjectInTable(node, objectName);
    if (objectEntry != NULL)
    {
        fdEntry = objectEntry->entries;
        while (fdEntry)
        {
            if (fdEntry->state == 0)
            {
                sendObjectMessage(fdEntry->fd, objectName);
            }
        }

        // apagar entrada na tabela e
        removeEntryFromInterestTable(node, objectName);
        // guardar o objeto em cahe
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
        fdEntry = findFdEntryInEntries(objectEntry, fd);
        fdEntry->state = 2; // estado fechado
    }
}
