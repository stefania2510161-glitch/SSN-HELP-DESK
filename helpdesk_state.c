#include "helpdesk_state.h"

TicketNode* ticketBSTRoot = NULL;
User users[MAX_USERS];
Engineer engineers[MAX_ENGINEERS];
int ticketCount = 0;
int userCount = 0;
int engineerCount = 0;

const char *issueNames[] = { "Furniture", "WiFi", "Network", "Hardware", "Software", "Other" };
const char *statusNames[] = { "Open", "Assigned", "In Progress", "Resolved", "Closed" };
