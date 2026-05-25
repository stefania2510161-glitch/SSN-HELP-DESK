#ifndef TICKET_SERVICE_H
#define TICKET_SERVICE_H

#include "helpdesk.h"

void loadTickets(void);
int syncTickets(void);
Ticket* createTicket(int uid, const char* issueType, const char* description, int priority, const char* notes, const char* imagePath);
void closeTicket(Ticket* ticket);
int reassignTicket(int ticketId, int manualEid);
int hardDeleteTicket(int uid, int tid);

#endif
