#ifndef UTILSFOROBJECT_H
#define UTILSFOROBJECT_H

#include "node.h"

void sendInterestMessage(int fd, char *objectName);
void sendObjectMessage(int fd, char *objectName);
void sendAbsenceObjectMessage(int fd, char *objectName);
TableInfo *findFdEntryInEntries(TableInfo *object, int fd);
interestTable *findObjectInTable(Node *node, char *objectName);
void sendInterestMessageToallInterface(Node *node, char *objectName, interestTable *objectEntry, int fd);
void removeEntryFromInterestTable(Node *node, char *objectName);
void createEntryToObjectList(int fd, int state, interestTable *entry);
interestTable *createEntryToInterestTable(Node *node, char *objectName);
void showInterestTable(Node *node);

#endif