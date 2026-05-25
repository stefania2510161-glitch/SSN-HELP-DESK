#ifndef JSON_OUTPUT_H
#define JSON_OUTPUT_H

#include <stddef.h>

#include "helpdesk.h"

void jsonEscapeStr(char* safeOut, const char* rawIn, int maxLen);
void fmtTime(char* timeBuffer, size_t bufferSize, time_t rawTime);
void printTicketJSON(const Ticket* ticket);
void printEngineerJSON(const Engineer* e);

#endif
