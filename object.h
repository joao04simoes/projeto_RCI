#ifndef OBJECT_H
#define OBJECT_H

#include "node.h"
#include "utils.h"

void retrieveObject(Node *node, char *objectName);
void sendInterestMessage(int fd, char *objectName);
void createEntryToInterestTable(Node *node, int fd, int state, char *objectName);
void AddEntryToInterestTable(Node *node, interestTable *entry);

#endif // OBJECT_H