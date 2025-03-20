#ifndef CACHE_H
#define CACHE_H
#include "node.h"
void initCache(Node *node, int size);
void addToCache(Node *node, char *data);
void printCache(Node *node);

#endif // UTILS_H