#include "engineer_service.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "helpdesk_state.h"
#include "stack.h"
#include "ticket_service.h"

void loadEngineers(void) {
    FILE* f = fopen("engineers.db", "rb");
    if (!f) return;
    if (fread(&engineerCount, sizeof(int), 1, f) != 1 || engineerCount < 0 || engineerCount > MAX_ENGINEERS) {
        engineerCount = 0;
        fclose(f);
        return;
    }
    if (fread(engineers, sizeof(Engineer), engineerCount, f) != (size_t)engineerCount) {
        engineerCount = 0;
    }
    fclose(f);
}

int syncEngineers(void) {
    FILE* f = fopen("engineers.db", "wb");
    if (!f) return -1;
    fwrite(&engineerCount, sizeof(int), 1, f);
    fwrite(engineers, sizeof(Engineer), engineerCount, f);
    fclose(f);
    return 0;
}

void initUsersDefault(void) {
    users[0] = (User){1,  "user",  "pass", STAFF,   "Normal User"};
    users[1] = (User){2,  "admin", "pass", MANAGER, "Administrator"};
    users[2] = (User){3,  "middleman", "pass", MIDDLEMAN, "Ticket Middleman"};
    users[3] = (User){101, "ravi", "pass", ENGINEER, "Ravi Kumar"};
    userCount = 4;
}

void initEngineersDefault(void) {
    engineers[0] = (Engineer){101, "Ravi Kumar", WIFI, 0, 0};
    engineerCount = 1;
    syncEngineers();
}

void resetSingleEngineerState(void) {
    engineers[0] = (Engineer){101, "Ravi Kumar", WIFI, 0, 0};
    engineerCount = 1;
    syncEngineers();
}

void recalcEngineersFromTickets(void) {
    engineers[0].ticketsAssigned = 0;
    engineers[0].ticketsResolved = 0;

    TicketNodeStack stack;
    ticket_stack_init(&stack, MAX_TICKETS_HEAP);

    TicketNode* current = ticketBSTRoot;
    while (current != NULL || !ticket_stack_is_empty(&stack)) {
        while (current != NULL) {
            ticket_stack_push(&stack, current);
            current = current->leftChild;
        }
        current = ticket_stack_pop(&stack);

        if (current->data.eid == engineers[0].id) {
            if (current->data.status == CLOSED || current->data.status == RESOLVED) {
                engineers[0].ticketsResolved++;
            } else {
                engineers[0].ticketsAssigned++;
            }
        }

        current = current->rightChild;
    }

    ticket_stack_free(&stack);
    syncEngineers();
}

void normalizeTicketsToSingleEngineer(void) {
    TicketNodeStack stack;
    ticket_stack_init(&stack, MAX_TICKETS_HEAP);

    TicketNode* current = ticketBSTRoot;
    while (current != NULL || !ticket_stack_is_empty(&stack)) {
        while (current != NULL) {
            ticket_stack_push(&stack, current);
            current = current->leftChild;
        }
        current = ticket_stack_pop(&stack);

        current->data.eid = engineers[0].id;
        if (current->data.status == OPEN) {
            current->data.status = ASSIGNED;
            current->data.timeAssigned = time(NULL);
        }

        current = current->rightChild;
    }

    ticket_stack_free(&stack);
    syncTickets();
    recalcEngineersFromTickets();
}

void normalizeEngineersAndAssignments(void) {
    if (engineerCount != 1 || engineers[0].id != 101 || strcmp(engineers[0].name, "Ravi Kumar") != 0) {
        resetSingleEngineerState();
    }

    normalizeTicketsToSingleEngineer();
}

void assignEngineer(Ticket* ticket) {
    int bestEngineerIndex = -1;
    int minimumWorkload = INT_MAX;

    for (int i = 0; i < engineerCount; i++) {
        if (engineers[i].specialty == ticket->issueType && engineers[i].ticketsAssigned < minimumWorkload) {
            minimumWorkload = engineers[i].ticketsAssigned;
            bestEngineerIndex = i;
        }
    }
    if (bestEngineerIndex == -1) {
        minimumWorkload = INT_MAX;
        for (int i = 0; i < engineerCount; i++) {
            if (engineers[i].ticketsAssigned < minimumWorkload) {
                minimumWorkload = engineers[i].ticketsAssigned;
                bestEngineerIndex = i;
            }
        }
    }
    if (bestEngineerIndex == -1) return;

    ticket->eid = engineers[bestEngineerIndex].id;
    ticket->status = ASSIGNED;
    ticket->timeAssigned = time(NULL);
    engineers[bestEngineerIndex].ticketsAssigned++;
}

int authenticateUser(char* username, char* password) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            return i;
        }
    }
    return -1;
}
