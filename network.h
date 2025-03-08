#ifndef NETWORK_H
#define NETWORK_H

#include "node.h"

void JoinNet(Node *node, char *Net);
void directJoin(Node *node, char *connectIP, int connectTCP);

#endif // NETWORK_H
