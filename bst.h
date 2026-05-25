#ifndef BST_H
#define BST_H

#include <stdio.h>

#include "helpdesk.h"

TicketNode* createNewTicketNode(Ticket newTicket);
void freeBST(TicketNode* root);
TicketNode* insertTicketIntoBST(TicketNode* root, Ticket newTicket);
TicketNode* removeTicketFromBST(TicketNode* root, int targetId);
TicketNode* searchTicketInBST(TicketNode* root, int targetId);
void writeTicketsToDiskRecursive(TicketNode* root, FILE* file);
void printTicketsInOrder(TicketNode* root, int targetUid, int* isFirstElement);

#endif
