#ifndef ENGINEER_SERVICE_H
#define ENGINEER_SERVICE_H

#include "helpdesk.h"

void loadEngineers(void);
int syncEngineers(void);
void initUsersDefault(void);
void initEngineersDefault(void);
void resetSingleEngineerState(void);
void recalcEngineersFromTickets(void);
void normalizeTicketsToSingleEngineer(void);
void normalizeEngineersAndAssignments(void);
void assignEngineer(Ticket* ticket);
int authenticateUser(char* username, char* password);

#endif
