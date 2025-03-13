#ifndef UTILS_H
#define UTILS_H

#include "node.h"

// Checks if a node is already connected to a given IP and port
int isInternal(Node *node, char *ip, int port);

// Error handling function (can be expanded)
void errorExit(const char *msg);
void executeCommand(char *command, Node *node);
void ExitNdn(Node *node);
void MakeNetList(char *buffer, Node *node);
void leaveNet(Node *node);
void excuteCommandFromBuffer(char *buffer, Node *node, int fd);

#endif // UTILS_H
