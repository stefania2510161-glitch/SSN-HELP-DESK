#ifndef STACK_H
#define STACK_H

#include "helpdesk.h"

typedef struct {
    TicketNode** items;
    int size;
    int capacity;
} TicketNodeStack;

void ticket_stack_init(TicketNodeStack* stack, int capacity);
void ticket_stack_free(TicketNodeStack* stack);
int ticket_stack_push(TicketNodeStack* stack, TicketNode* node);
TicketNode* ticket_stack_pop(TicketNodeStack* stack);
int ticket_stack_is_empty(const TicketNodeStack* stack);

#endif
