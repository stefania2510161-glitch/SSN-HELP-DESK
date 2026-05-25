#include "json_output.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "helpdesk_state.h"

void jsonEscapeStr(char* safeOut, const char* rawIn, int maxLen) {
    int outIndex = 0;
    for (int i = 0; rawIn[i] != '\0' && outIndex < maxLen - 4; i++) {
        switch (rawIn[i]) {
            case '"':
                safeOut[outIndex++] = '\\';
                safeOut[outIndex++] = '"';
                break;
            case '\\':
                safeOut[outIndex++] = '\\';
                safeOut[outIndex++] = '\\';
                break;
            case '\n':
                safeOut[outIndex++] = '\\';
                safeOut[outIndex++] = 'n';
                break;
            case '\r':
                safeOut[outIndex++] = '\\';
                safeOut[outIndex++] = 'r';
                break;
            case '\t':
                safeOut[outIndex++] = '\\';
                safeOut[outIndex++] = 't';
                break;
            default:
                safeOut[outIndex++] = rawIn[i];
                break;
        }
    }
    safeOut[outIndex] = '\0';
}

void fmtTime(char* timeBuffer, size_t bufferSize, time_t rawTime) {
    if (rawTime <= 0) {
        snprintf(timeBuffer, bufferSize, "null");
        return;
    }

    struct tm* timeStruct = localtime(&rawTime);
    snprintf(timeBuffer, bufferSize, "\"%04d-%02d-%02dT%02d:%02d:%02d\"",
             timeStruct->tm_year + 1900, timeStruct->tm_mon + 1, timeStruct->tm_mday,
             timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec);
}

void printTicketJSON(const Ticket* ticket) {
    char safeDesc[MAX_DESCRIPTION_LEN * 2];
    char safeNotes[MAX_NOTES_LEN * 2];
    char timeCreated[40];
    char timeAssigned[40];
    char timeClosed[40];
    char engineerIdStr[16];

    int issueIndex = (ticket->issueType >= 0 && ticket->issueType < (int)(sizeof(issueNames) / sizeof(issueNames[0]))) ? ticket->issueType : OTHER;
    int statusIndex = (ticket->status >= 0 && ticket->status < (int)(sizeof(statusNames) / sizeof(statusNames[0]))) ? ticket->status : OPEN;

    jsonEscapeStr(safeDesc, ticket->description, sizeof(safeDesc));
    jsonEscapeStr(safeNotes, ticket->notes, sizeof(safeNotes));
    fmtTime(timeCreated, sizeof(timeCreated), ticket->timeCreated);
    fmtTime(timeAssigned, sizeof(timeAssigned), ticket->timeAssigned);
    fmtTime(timeClosed, sizeof(timeClosed), ticket->timeClosed);

    if (ticket->eid < 0) snprintf(engineerIdStr, sizeof(engineerIdStr), "null");
    else snprintf(engineerIdStr, sizeof(engineerIdStr), "%d", ticket->eid);

    printf("{\"id\":%d,\"user_id\":%d,\"engineer_id\":%s,\"issue_type\":\"%s\",\"status\":\"%s\",\"description\":\"%s\",\"notes\":\"%s\",\"created_at\":%s,\"assigned_at\":%s,\"closed_at\":%s,\"priority\":%d}",
           ticket->id, ticket->uid, engineerIdStr, issueNames[issueIndex], statusNames[statusIndex],
           safeDesc, safeNotes, timeCreated, timeAssigned, timeClosed, ticket->priority);
}

void printEngineerJSON(const Engineer* e) {
    char safeName[MAX_USERNAME_LEN * 2];
    const char* specialtyName = (e->specialty >= 0 && e->specialty < (int)(sizeof(issueNames) / sizeof(issueNames[0]))) ? issueNames[e->specialty] : "Other";

    jsonEscapeStr(safeName, e->name, sizeof(safeName));
    printf("{\"id\":%d,\"name\":\"%s\",\"specialty\":\"%s\",\"active_tickets\":%d,\"total_resolved\":%d}",
           e->id, safeName, specialtyName, e->ticketsAssigned, e->ticketsResolved);
}
