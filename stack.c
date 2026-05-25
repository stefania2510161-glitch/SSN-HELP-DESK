#include "stack.h"

#include <stdlib.h>

void ticket_stack_init(TicketNodeStack* stack, int capacity) {
    stack->items = (TicketNode**)malloc((size_t)capacity * sizeof(TicketNode*));
    stack->size = 0;
    stack->capacity = capacity;
}

void ticket_stack_free(TicketNodeStack* stack) {
    free(stack->items);
    stack->items = NULL;
    stack->size = 0;
    stack->capacity = 0;
}

int ticket_stack_push(TicketNodeStack* stack, TicketNode* node) {
    if (stack->size >= stack->capacity) return 0;
    stack->items[stack->size++] = node;
    return 1;
}

TicketNode* ticket_stack_pop(TicketNodeStack* stack) {
    if (stack->size <= 0) return NULL;
    return stack->items[--stack->size];
}

int ticket_stack_is_empty(const TicketNodeStack* stack) {
    return stack->size == 0;
}
