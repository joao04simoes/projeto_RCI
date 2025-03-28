#ifndef UTILS_H
#define UTILS_H

#include "node.h"

int isValidIP(const char *ip);
int isValidPort(const char *port);
int isInternal(Node *node, char *ip, int port);
void executeCommand(char *command, Node *node);
void ExitNdn(Node *node);
void MakeNetList(char *buffer, Node *node);
void leaveNet(Node *node);
void excuteCommandFromBuffer(char *buffer, Node *node, int fd);
void showNames(Node *node);
NodeList *randomNode(NodeList *nodeList);
void memCleanup(Node *node);
void addInfoToNode(Info *info, char *ip, int port, int fd);
void SendSafeMsg(char *ip, int port, int FD);
void SendEntryMsg(char *ip, int port, int FD);
void addInternalNeighbor(Node *node, int fd, char *ip, int port);
void removeInternalNeighbor(Node *node, int fd);
void AddNodeFromNetList(Node *node, char *ip, int port);

#endif // UTILS_H
