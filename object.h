#ifndef OBJECT_H
#define OBJECT_H

#include "node.h"
#include "utils.h"

void retrieveObject(Node *node, char *objectName);
void sendInterestMessage(int fd, char *objectName);
void addObjectToList(Node *node, char *objectName);
char *findObjectInLIst(Node *node, char *objectName);
void handleObjectMessage(Node *node, char *objectName);
void AddEntryToInterestTable(Node *node, interestTable *entry);
void handleInterest(Node *node, int fd, char *objectName);
void handleAbsenceMessage(Node *node, int fd, char *objectName);
char *findObjectInCache(Node *node, char *objectName);
void deleteObject(Node *node, char *objectName);

#endif // OBJECT_H