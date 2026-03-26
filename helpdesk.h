#ifndef HELPDESK_H
#define HELPDESK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TICKETS 1000
#define MAX_USERS 500
#define MAX_ENGINEERS 100
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_DESCRIPTION_LEN 256
#define MAX_NOTES_LEN 512
#define CSV_BUFFER_SIZE 2048

// Enums
enum IssueType {
    FURNITURE = 0,
    WIFI = 1,
    NETWORK = 2,
    HARDWARE = 3,
    SOFTWARE = 4,
    OTHER = 5
};

enum TicketStatus {
    OPEN = 0,
    ASSIGNED = 1,
    IN_PROGRESS = 2,
    RESOLVED = 3,
    CLOSED = 4
};

enum UserRole {
    STAFF = 0,
    ENGINEER = 1,
    MANAGER = 2
};

// Structures
typedef struct {
    int id;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    enum UserRole role;
    char name[MAX_USERNAME_LEN];
} User;

typedef struct {
    int id;
    char name[MAX_USERNAME_LEN];
    enum IssueType specialty;
    int ticketsAssigned;
    int ticketsResolved;
} Engineer;

typedef struct {
    time_t timeCreated;
    time_t timeAssigned;
    time_t timeClosed;
    enum IssueType issueType;
    char description[MAX_DESCRIPTION_LEN];
    int id;
    int uid;
    int eid;
    enum TicketStatus status;
    char notes[MAX_NOTES_LEN];
} Ticket;

// Database structure
typedef struct {
    Ticket tickets[MAX_TICKETS];
    int ticketCount;
    User users[MAX_USERS];
    int userCount;
    Engineer engineers[MAX_ENGINEERS];
    int engineerCount;
    int nextTicketID;
} Database;


int init(Database* db);
int syncTickets(Database* db);

// User related operations
int authenticateUser(Database* db, const char* username, const char* password, User* outUser);
User* getUserById(Database* db, int userId);
int addUser(Database* db, const char* username, const char* password,
            enum UserRole role, const char* name);

// Ticket related operations
Ticket* createTicket(Database* db, int uid, enum IssueType issue, const char* description);
Ticket* getTicketById(Database* db, int ticketId);
Ticket** getTicketsForUser(Database* db, int uid, int* count);
Ticket** getTicketsForEngineer(Database* db, int eid, int* count);
Ticket** getAllOpenTickets(Database* db, int* count);
void updateTicketStatus(Database* db, int ticketId, enum TicketStatus status);
void addTicketNote(Database* db, int ticketId, const char* note);
void closeTicket(Database* db, Ticket* ticket);

// Engineer related operations
int genTicketID(Database* db);
void assignEngineer(Database* db, Ticket* ticket);
Engineer* getEngineerById(Database* db, int engineerId);
Engineer** getAllEngineers(Database* db, int* count);

// Report generation
void generateReport(Database* db);
int getTicketCountByStatus(Database* db, enum TicketStatus status);
int getTicketCountByIssueType(Database* db, enum IssueType type);
double getAverageResolutionTime(Database* db);

// Helper functions
const char* getIssueName(enum IssueType issue);
enum IssueType parseIssueType(const char* issueStr);
const char* getStatusName(enum TicketStatus status);
const char* getRoleName(enum UserRole role);
char* escapeCSV(const char* str, char* buffer);
char* unescapeCSV(const char* str, char* buffer);

#endif