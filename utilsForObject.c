#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilsForObject.h"

// encontra a entrada de uma interface fd numa lista de entradas do objeto
TableInfo *findFdEntryInEntries(TableInfo *objectEntries, int fd, int state)
{
    if (!objectEntries)
        return NULL;

    TableInfo *curr = objectEntries;
    while (curr)
    {
        if (curr->fd == fd && curr->state == state)
        {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

// encotra um objeto na tabela de interesse
interestTable *findObjectInTable(Node *node, char *objectName)
{
    interestTable *curr = node->Table;
    while (curr)
    {
        if (strcmp(curr->objectName, objectName) == 0)
        {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

// envia mensagem de interesse para todos os interfaces exceto a interface fd
void sendInterestMessageToallInterface(Node *node, char *objectName, interestTable *objectEntry, int fd)
{
    NodeList *curr = node->intr;
    if ((fd > 0 && node->vzext.FD != fd) || fd == -1) // envia para o externo
    {
        sendInterestMessage(node->vzext.FD, objectName);
        createEntryToObjectList(node->vzext.FD, 1, objectEntry);
    }

    while (curr)
    {
        if (node->vzext.FD != curr->data.FD && curr->data.FD != fd) // não envia para o externos se este for interno
        {
            sendInterestMessage(curr->data.FD, objectName);
            createEntryToObjectList(curr->data.FD, 1, objectEntry);
        }
        curr = curr->next;
    }
}

// remove uma entrada da tabela de interesse
void removeEntryFromInterestTable(Node *node, char *objectName)
{
    if (node == NULL || node->Table == NULL)
    {
        return;
    }

    TableInfo *curr, *tmp;
    interestTable *prev = NULL, *currTable = node->Table;

    // Procurar a entrada na tabela
    while (currTable)
    {
        if (strcmp(currTable->objectName, objectName) == 0)
        {
            break;
        }
        prev = currTable;
        currTable = currTable->next;
    }

    // Se não encontrou o objeto, sair da função
    if (currTable == NULL)
    {
        return;
    }

    curr = currTable->entries;
    while (curr)
    {
        tmp = curr;
        curr = curr->next;
        free(tmp);
    }

    if (prev == NULL)
    {
        // O objeto a remover está no início da tabela
        node->Table = currTable->next;
    }
    else
    {
        prev->next = currTable->next;
    }

    free(currTable);
}

// cria uma entrada de uma interface  na lista de interfaces do objeto
void createEntryToObjectList(int fd, int state, interestTable *entry)
{
    TableInfo *newEntry = (TableInfo *)malloc(sizeof(TableInfo));
    newEntry->fd = fd;
    newEntry->state = state;
    newEntry->next = entry->entries;
    entry->entries = newEntry;
}

// cria uma entrada de um objeto na tabela de interesses
interestTable *createEntryToInterestTable(Node *node, char *objectName)
{

    interestTable *newEntry = (interestTable *)malloc(sizeof(interestTable));
    strcpy(newEntry->objectName, objectName);
    newEntry->next = node->Table;
    node->Table = newEntry;
    node->Table->entries = NULL;
    return newEntry;
}

// Print a tabela de interesse
void showInterestTable(Node *node)
{
    interestTable *curr = node->Table;
    TableInfo *currEntry;
    printf("Interest Table:\n");
    while (curr)
    {
        printf("Object: %s\n", curr->objectName);
        currEntry = curr->entries;
        while (currEntry)
        {
            printf("  FD: %d, State: %d\n", currEntry->fd, currEntry->state);
            currEntry = currEntry->next;
        }
        curr = curr->next;
    }
}

// adiciona um objeto à lista de objetos
void addObjectToList(Node *node, char *objectName)
{
    Names *newObject = (Names *)malloc(sizeof(Names));
    strcpy(newObject->name, objectName);
    newObject->next = node->Objects;
    node->Objects = newObject;
}

// encontra um objeto na lista de objetos
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

// encontra um objeto na cache
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

// remove um objeto da lista de objetos
void deleteObject(Node *node, char *objectName)
{

    Names *curr = node->Objects, *prev = NULL;
    while (curr)
    {
        if (strcmp(curr->name, objectName) == 0)
        {
            if (prev == NULL)
                node->Objects = curr->next;
            else
                prev->next = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

// envia mensagem de interesse
void sendInterestMessage(int fd, char *objectName)
{
    char buffer[128];
    sprintf(buffer, "INTEREST %s\n", objectName);
    write(fd, buffer, strlen(buffer));
}

// envia mensagem de objeto
void sendObjectMessage(int fd, char *objectName)
{
    char buffer[128];
    sprintf(buffer, "OBJECT %s\n", objectName);
    write(fd, buffer, strlen(buffer));
}

// envia mensagem de ausência de objeto
void sendAbsenceObjectMessage(int fd, char *objectName)
{
    char buffer[128];
    sprintf(buffer, "NOOBJECT %s\n", objectName);
    write(fd, buffer, strlen(buffer));
}