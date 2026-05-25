#ifndef HELPDESK_STATE_H
#define HELPDESK_STATE_H

#include "helpdesk.h"

extern TicketNode* ticketBSTRoot;
extern User users[MAX_USERS];
extern Engineer engineers[MAX_ENGINEERS];
extern int ticketCount;
extern int userCount;
extern int engineerCount;
extern const char* issueNames[6];
extern const char* statusNames[5];

#endif
