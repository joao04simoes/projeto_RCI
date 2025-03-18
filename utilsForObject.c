#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilsForObject.h"

void sendInterestMessage(int fd, char *objectName)
{
    char buffer[128];
    sprintf(buffer, "INTEREST %s\n", objectName);
    write(fd, buffer, strlen(buffer));
}
void sendObjectMessage(int fd, char *objectName)
{
    char buffer[128];
    sprintf(buffer, "OBJECT %s\n", objectName);
    write(fd, buffer, strlen(buffer));
}
void sendAbsenceObjectMessage(int fd, char *objectName)
{
    char buffer[128];
    sprintf(buffer, "NOOBJECT %s\n", objectName);
    write(fd, buffer, strlen(buffer));
}

TableInfo *findFdEntryInEntries(TableInfo *object, int fd)
{
    TableInfo *curr = object;
    while (curr)
    {
        if (curr->fd == fd)
        {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

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

void sendInterestMessageToallInterface(Node *node, char *objectName, interestTable *objectEntry, int fd)
{
    NodeList *curr = node->intr;
    sendInterestMessage(node->vzext.FD, objectName);
    createEntryToObjectList(node->vzext.FD, 1, objectEntry);

    while (curr) // passar isto para uma função
    {
        if (curr->data.FD && node->vzext.FD != curr->data.FD && curr->data.FD != fd) // verificar se manda para o externo
        {
            sendInterestMessage(curr->data.FD, objectName);
            createEntryToObjectList(curr->data.FD, 1, objectEntry);
        }
        curr = curr->next;
    }
}

void removeEntryFromInterestTable(Node *node, char *objectName)
{
    TableInfo *curr, *tmp;
    interestTable *prev = NULL, *currTable;

    while (currTable)
    {
        if (strcmp(currTable->objectName, objectName) == 0)
        {
            break;
        }
        prev = currTable;
        currTable = currTable->next;
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
        node->Table = currTable->next;
        free(currTable);
    }
    else
    {
        prev->next = currTable->next;
        free(currTable);
    }
}
void createEntryToObjectList(int fd, int state, interestTable *entry)
{
    TableInfo *newEntry = (TableInfo *)malloc(sizeof(TableInfo));
    newEntry->fd = fd;
    newEntry->state = state;
    entry->entries = newEntry;
}

interestTable *createEntryToInterestTable(Node *node, char *objectName)
{
    TableInfo *fdEntry;
    interestTable *newEntry = (interestTable *)malloc(sizeof(interestTable));
    strcpy(newEntry->objectName, objectName);
    newEntry->next = node->Table;
    node->Table = newEntry;
    return newEntry;
}
