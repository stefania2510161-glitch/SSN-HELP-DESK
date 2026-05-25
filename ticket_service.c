#include "ticket_service.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "bst.h"
#include "engineer_service.h"
#include "helpdesk_state.h"

static int compareIgnoreCase(const char* a, const char* b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static int genTicketID(void) {
    int lastID = 0;
    FILE* f = fopen("ticket_counter.dat", "rb");
    if (f) {
        fread(&lastID, sizeof(int), 1, f);
        fclose(f);
    }

    lastID++;
    f = fopen("ticket_counter.dat", "wb");
    if (f) {
        fwrite(&lastID, sizeof(int), 1, f);
        fclose(f);
    }

    return lastID;
}

static enum IssueType parseIssueType(const char* s) {
    for (int i = 0; i < (int)(sizeof(issueNames) / sizeof(issueNames[0])); i++) {
        if (compareIgnoreCase(issueNames[i], s)) return (enum IssueType)i;
    }
    return OTHER;
}

void loadTickets(void) {
    FILE* file = fopen("tickets.db", "rb");
    if (!file) return;

    int savedTicketCount = 0;
    if (fread(&savedTicketCount, sizeof(int), 1, file) != 1 || savedTicketCount < 0 || savedTicketCount > MAX_TICKETS_HEAP) {
        fclose(file);
        return;
    }

    for (int i = 0; i < savedTicketCount; i++) {
        Ticket tempTicket;
        if (fread(&tempTicket, sizeof(Ticket), 1, file) != 1) break;

        tempTicket.description[MAX_DESCRIPTION_LEN - 1] = '\0';
        tempTicket.notes[MAX_NOTES_LEN - 1] = '\0';
        tempTicket.imagePath[255] = '\0';
        if (tempTicket.issueType < 0 || tempTicket.issueType >= (int)(sizeof(issueNames) / sizeof(issueNames[0]))) tempTicket.issueType = OTHER;
        if (tempTicket.status < 0 || tempTicket.status >= (int)(sizeof(statusNames) / sizeof(statusNames[0]))) tempTicket.status = OPEN;

        ticketBSTRoot = insertTicketIntoBST(ticketBSTRoot, tempTicket);
        ticketCount++;
    }

    fclose(file);
}

int syncTickets(void) {
    FILE* file = fopen("tickets.db", "wb");
    if (!file) return -1;
    fwrite(&ticketCount, sizeof(int), 1, file);
    writeTicketsToDiskRecursive(ticketBSTRoot, file);
    fclose(file);
    return 0;
}

Ticket* createTicket(int uid, const char* issueType, const char* description, int priority, const char* notes, const char* imagePath) {
    Ticket newTicket;
    newTicket.id = genTicketID();
    newTicket.uid = uid;
    newTicket.eid = -1;
    newTicket.issueType = parseIssueType(issueType);
    newTicket.status = OPEN;
    newTicket.priority = priority;
    newTicket.timeCreated = time(NULL);
    newTicket.timeAssigned = 0;
    newTicket.timeClosed = 0;

    strncpy(newTicket.description, description, MAX_DESCRIPTION_LEN - 1);
    newTicket.description[MAX_DESCRIPTION_LEN - 1] = '\0';

    strncpy(newTicket.notes, notes, MAX_NOTES_LEN - 1);
    newTicket.notes[MAX_NOTES_LEN - 1] = '\0';

    strncpy(newTicket.imagePath, imagePath, 255);
    newTicket.imagePath[255] = '\0';

    ticketBSTRoot = insertTicketIntoBST(ticketBSTRoot, newTicket);
    ticketCount++;
    syncTickets();
    syncEngineers();

    TicketNode* insertedNode = searchTicketInBST(ticketBSTRoot, newTicket.id);
    return &(insertedNode->data);
}

void closeTicket(Ticket* ticket) {
    if (ticket->status == CLOSED) return;
    ticket->status = CLOSED;
    ticket->timeClosed = time(NULL);

    for (int i = 0; i < engineerCount; i++) {
        if (engineers[i].id == ticket->eid) {
            engineers[i].ticketsResolved++;
            if (engineers[i].ticketsAssigned > 0) engineers[i].ticketsAssigned--;
            break;
        }
    }

    syncTickets();
    syncEngineers();
}

int reassignTicket(int ticketId, int manualEid) {
    TicketNode* targetNode = searchTicketInBST(ticketBSTRoot, ticketId);
    if (targetNode == NULL) return -1;

    Ticket* ticket = &(targetNode->data);

    for (int j = 0; j < engineerCount; j++) {
        if (engineers[j].id == ticket->eid) {
            if (engineers[j].ticketsAssigned > 0) engineers[j].ticketsAssigned--;
            break;
        }
    }

    if (manualEid > 0) {
        ticket->eid = manualEid;
        ticket->status = ASSIGNED;
        ticket->timeAssigned = time(NULL);
        for (int i = 0; i < engineerCount; i++) {
            if (engineers[i].id == manualEid) {
                engineers[i].ticketsAssigned++;
                break;
            }
        }
    } else {
        ticket->eid = -1;
        ticket->status = OPEN;
        assignEngineer(ticket);
    }

    syncTickets();
    syncEngineers();
    return 0;
}
