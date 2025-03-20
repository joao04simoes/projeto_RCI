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
    if (node == NULL || node->Table == NULL)
    {
        printf("Erro: Tabela vazia ou nó inválido\n");
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
        printf("Erro: Objeto '%s' não encontrado na tabela\n", objectName);
        return;
    }

    // Liberar todas as entradas associadas ao objeto
    curr = currTable->entries;
    while (curr)
    {
        tmp = curr;
        curr = curr->next;
        free(tmp);
    }

    // Remover a entrada da tabela de interesse
    if (prev == NULL)
    {
        // O objeto a remover está no início da tabela
        node->Table = currTable->next;
        printf("first object\n");
    }
    else
    {
        prev->next = currTable->next;
    }

    free(currTable);
}

void createEntryToObjectList(int fd, int state, interestTable *entry)
{
    TableInfo *newEntry = (TableInfo *)malloc(sizeof(TableInfo));
    newEntry->fd = fd;
    newEntry->state = state;
    newEntry->next = entry->entries; // Maintain the linked list
    entry->entries = newEntry;
}
interestTable *createEntryToInterestTable(Node *node, char *objectName)
{

    interestTable *newEntry = (interestTable *)malloc(sizeof(interestTable));
    strcpy(newEntry->objectName, objectName);
    newEntry->next = node->Table;
    node->Table = newEntry;
    node->Table->entries = NULL;
    return newEntry;
}

void showInterestTable(Node *node)
{
    interestTable *curr = node->Table;
    TableInfo *currEntry;
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
