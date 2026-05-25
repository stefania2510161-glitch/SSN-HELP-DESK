#ifndef HEAP_H
#define HEAP_H

#include "helpdesk.h"

void insertMinHeap(Ticket** heap, int* size, Ticket* t);
Ticket* extractMin(Ticket** heap, int* size);
void collectTicketsForHeap(TicketNode* root, Ticket** heap, int* size);
void printUnassignedTickets(TicketNode* root, int* isFirstElement);

#endif
