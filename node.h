#ifndef NODE_H
#define NODE_H

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

// Node Information
typedef struct Info
{
    int port;
    char ip[20];
    int FD;
} Info;

// Linked list of internal neighbors
typedef struct NodeList
{
    Info data;
    struct NodeList *next;
} NodeList;

typedef struct TableInfo
{
    int fd;
    int state; // 0 reponse 1 await 2 closed
    struct TableInfo *next
} TableInfo;

typedef struct interestTable
{
    char objectName[128];
    struct TableInfo *entries;
    struct interestTable *next;
} interestTable;

typedef struct Names
{
    char name[101];
    struct Names *next;
} Names;

// Node structure
typedef struct
{
    Info vzext;
    NodeList *intr;
    Info vzsalv;
    int port;
    char ip[20];
    int FD;
    NodeList *netlist;
    int NetReg;
    char NET[4];
    char regIP[20];
    char regUDP[6];
    interestTable *Table;
    Names *Objects;
    char *cache;

} Node;

// Function prototypes
void addInternalNeighbor(Node *node, int fd, char *ip, int port);
void removeInternalNeighbor(Node *node, int fd);
void updateInternalsSafe(Node *node);
void handleEntry(Node *node, int newfd, char *ip, int port);
void handleSafe(Node *node, char *ip, int port);
void addInfoToNode(Info *info, char *ip, int port, int fd);
void verifyExternal(Node *node);
void SendSafeMsg(char *ip, int port, int FD);
void SendEntryMsg(char *ip, int port, int FD);
void AddNodeFromNetList(Node *node, char *ip, int port);

#endif // NODE_H
