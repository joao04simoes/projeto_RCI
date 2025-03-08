#ifndef UTILS_H
#define UTILS_H

#include "node.h"

// Checks if a node is already connected to a given IP and port
int isInternal(Node *node, char *ip, int port);

// Error handling function (can be expanded)
void errorExit(const char *msg);
void executeCommand(char *command, Node *node);
void ExitNdn(Node *node);

#endif // UTILS_H
