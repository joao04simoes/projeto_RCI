#ifndef OBJECT_H
#define OBJECT_H

#include "node.h"
#include "utils.h"

void retrieveObject(Node *node, char *objectName);
void handleInterest(Node *node, int fd, char *objectName);
void handleObjectMessage(Node *node, char *objectName);
void handleAbsenceMessage(Node *node, int fd, char *objectName);
int sendAbsenceIfNoInterest(interestTable *objectEntry);
void deleteObject(Node *node, char *objectName);

#endif // OBJECT_H