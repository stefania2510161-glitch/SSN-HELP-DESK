#include "commands.h"

#include <stdio.h>
#include <string.h>

#include "bst.h"
#include "engineer_service.h"
#include "heap.h"
#include "helpdesk_state.h"
#include "json_output.h"
#include "ticket_service.h"

static int cmdLogin(int argc, char* argv[]) {
    if (argc < 4) {
        printf("{\"success\":false,\"error\":\"Usage: login <username> <password>\"}\n");
        return 1;
    }

    int idx = authenticateUser(argv[2], argv[3]);
    if (idx < 0) {
        printf("{\"success\":false,\"error\":\"Invalid credentials\"}\n");
        return 1;
    }

    char safeUname[MAX_USERNAME_LEN * 2];
    char safeFullname[MAX_USERNAME_LEN * 2];
    jsonEscapeStr(safeUname, users[idx].username, sizeof(safeUname));
    jsonEscapeStr(safeFullname, users[idx].name, sizeof(safeFullname));

    const char* role = users[idx].role == MANAGER ? "admin" : users[idx].role == MIDDLEMAN ? "middleman" : "user";
    printf("{\"success\":true,\"user_id\":%d,\"username\":\"%s\",\"full_name\":\"%s\",\"role\":\"%s\"}\n",
           users[idx].id, safeUname, safeFullname, role);
    return 0;
}

static int cmdCreateTicket(int argc, char* argv[]) {
    if (argc < 8) {
        printf("{\"success\":false,\"error\":\"Insufficient arguments for create_ticket\"}\n");
        return 1;
    }

    int uid = atoi(argv[2]);
    Ticket* t = createTicket(uid, argv[3], argv[4], atoi(argv[5]), argv[6], argv[7]);
    if (!t) {
        printf("{\"success\":false,\"error\":\"System error\"}\n");
        return 1;
    }

    printf("{\"success\":true,\"ticket\":");
    printTicketJSON(t);
    printf("}\n");
    return 0;
}

static int cmdCloseTicket(int argc, char* argv[]) {
    if (argc < 3) {
        printf("{\"success\":false,\"error\":\"Usage: close_ticket <ticket_id>\"}\n");
        return 1;
    }

    int tid = atoi(argv[2]);
    TicketNode* targetNode = searchTicketInBST(ticketBSTRoot, tid);
    if (targetNode == NULL) {
        printf("{\"success\":false,\"error\":\"Ticket #%d not found\"}\n", tid);
        return 1;
    }

    if (targetNode->data.status == CLOSED) {
        printf("{\"success\":false,\"error\":\"Ticket already closed\"}\n");
        return 1;
    }

    closeTicket(&(targetNode->data));
    printf("{\"success\":true,\"message\":\"Ticket #%d closed\",\"ticket\":", tid);
    printTicketJSON(&(targetNode->data));
    printf("}\n");
    return 0;
}

static int cmdAssignTicket(int argc, char* argv[]) {
    if (argc < 3) {
        printf("{\"success\":false,\"error\":\"Usage: assign_ticket <ticket_id> [engineer_id]\"}\n");
        return 1;
    }

    int tid = atoi(argv[2]);
    int eid = (argc >= 4) ? atoi(argv[3]) : -1;
    if (reassignTicket(tid, eid) != 0) {
        printf("{\"success\":false,\"error\":\"Ticket #%d not found\"}\n", tid);
        return 1;
    }

    TicketNode* updatedNode = searchTicketInBST(ticketBSTRoot, tid);
    printf("{\"success\":true,\"ticket\":");
    printTicketJSON(&(updatedNode->data));
    printf("}\n");
    return 0;
}

static int cmdSetPriority(int argc, char* argv[]) {
    if (argc < 4) {
        printf("{\"success\":false,\"error\":\"Usage: set_priority <ticket_id> <priority>\"}\n");
        return 1;
    }

    int tid = atoi(argv[2]);
    int prio = atoi(argv[3]);
    TicketNode* targetNode = searchTicketInBST(ticketBSTRoot, tid);
    if (targetNode == NULL) {
        printf("{\"success\":false,\"error\":\"Ticket #%d not found\"}\n", tid);
        return 1;
    }

    targetNode->data.priority = prio;
    syncTickets();
    printf("{\"success\":true,\"ticket\":");
    printTicketJSON(&(targetNode->data));
    printf("}\n");
    return 0;
}

static int cmdAutoAssignTicket(int argc, char* argv[]) {
    if (argc < 3) {
        printf("{\"success\":false,\"error\":\"Usage: auto_assign_ticket <ticket_id>\"}\n");
        return 1;
    }

    int tid = atoi(argv[2]);
    TicketNode* targetNode = searchTicketInBST(ticketBSTRoot, tid);
    if (targetNode == NULL) {
        printf("{\"success\":false,\"error\":\"Ticket #%d not found\"}\n", tid);
        return 1;
    }

    assignEngineer(&(targetNode->data));
    syncTickets();
    syncEngineers();
    printf("{\"success\":true,\"ticket\":");
    printTicketJSON(&(targetNode->data));
    printf("}\n");
    return 0;
}

static int cmdSetNotes(int argc, char* argv[]) {
    if (argc < 4) {
        printf("{\"success\":false,\"error\":\"Usage: set_notes <ticket_id> <notes...>\"}\n");
        return 1;
    }

    int tid = atoi(argv[2]);
    TicketNode* targetNode = searchTicketInBST(ticketBSTRoot, tid);
    if (targetNode == NULL) {
        printf("{\"success\":false,\"error\":\"Ticket #%d not found\"}\n", tid);
        return 1;
    }

    targetNode->data.notes[0] = '\0';
    for (int i = 3; i < argc; i++) {
        if (i > 3) strncat(targetNode->data.notes, " ", MAX_NOTES_LEN - strlen(targetNode->data.notes) - 1);
        strncat(targetNode->data.notes, argv[i], MAX_NOTES_LEN - strlen(targetNode->data.notes) - 1);
    }
    syncTickets();
    printf("{\"success\":true}\n");
    return 0;
}

static int cmdEditTicket(int argc, char* argv[]) {
    if (argc < 5) {
        printf("{\"success\":false,\"error\":\"Usage: edit_ticket <uid> <tid> <new_desc>\"}\n");
        return 1;
    }

    int uid = atoi(argv[2]);
    int tid = atoi(argv[3]);
    TicketNode* node = searchTicketInBST(ticketBSTRoot, tid);
    if (node && node->data.uid == uid && node->data.status == OPEN) {
        strncpy(node->data.description, argv[4], MAX_DESCRIPTION_LEN - 1);
        node->data.description[MAX_DESCRIPTION_LEN - 1] = '\0';
        syncTickets();
        printf("{\"success\":true,\"ticket\":");
        printTicketJSON(&(node->data));
        printf("}\n");
        return 0;
    }

    printf("{\"success\":false,\"error\":\"Ticket not found or unauthorized\"}\n");
    return 1;
}

static int cmdDeleteTicket(int argc, char* argv[]) {
    if (argc < 4) {
        printf("{\"success\":false,\"error\":\"Usage: delete_ticket <uid> <tid>\"}\n");
        return 1;
    }

    int uid = atoi(argv[2]);
    int tid = atoi(argv[3]);
    TicketNode* node = searchTicketInBST(ticketBSTRoot, tid);
    if (node && node->data.uid == uid && node->data.status == OPEN) {
        node->data.status = CLOSED;
        syncTickets();
        printf("{\"success\":true}\n");
        return 0;
    }

    printf("{\"success\":false,\"error\":\"Ticket not found or unauthorized\"}\n");
    return 1;
}

static int cmdListTicketsAdmin(void) {
    Ticket* heap[MAX_TICKETS_HEAP];
    int heapSize = 0;
    collectTicketsForHeap(ticketBSTRoot, heap, &heapSize);

    int isFirstElement = 1;
    printf("{\"success\":true,\"tickets\":[");
    while (heapSize > 0) {
        Ticket* t = extractMin(heap, &heapSize);
        if (!isFirstElement) printf(",");
        printTicketJSON(t);
        isFirstElement = 0;
    }
    printUnassignedTickets(ticketBSTRoot, &isFirstElement);
    printf("]}\n");
    return 0;
}

static int cmdListTickets(int argc, char* argv[]) {
    int targetUid = (argc >= 3) ? atoi(argv[2]) : 0;
    int isFirstElement = 1;
    printf("{\"success\":true,\"tickets\":[");
    printTicketsInOrder(ticketBSTRoot, targetUid, &isFirstElement);
    printf("]}\n");
    return 0;
}

static int cmdSearchBST(int argc, char* argv[]) {
    if (argc < 3) {
        printf("{\"success\":false,\"error\":\"Usage: search_bst <ticket_id>\"}\n");
        return 1;
    }

    int tid = atoi(argv[2]);
    int path[100];
    int pathSize = 0;

    TicketNode* curr = ticketBSTRoot;
    while (curr != NULL) {
        path[pathSize++] = curr->data.id;
        if (curr->data.id == tid) break;
        if (tid < curr->data.id) curr = curr->leftChild;
        else curr = curr->rightChild;
    }

    printf("{\"success\":true,\"found\":%s,\"path\":[", (curr != NULL) ? "true" : "false");
    for (int i = 0; i < pathSize; i++) {
        printf("%d%s", path[i], (i == pathSize - 1) ? "" : ",");
    }
    printf("]");
    if (curr != NULL) {
        printf(",\"ticket\":");
        printTicketJSON(&(curr->data));
    }
    printf("}\n");
    return 0;
}

static int cmdListEngineers(void) {
    printf("{\"success\":true,\"engineers\":[");
    for (int i = 0; i < engineerCount; i++) {
        if (i > 0) printf(",");
        printEngineerJSON(&engineers[i]);
    }
    printf("]}\n");
    return 0;
}

int run_command(int argc, char* argv[]) {
    initUsersDefault();
    loadEngineers();
    if (engineerCount == 0) initEngineersDefault();
    loadTickets();
    normalizeEngineersAndAssignments();

    const char* cmd = argv[1];
    if (strcmp(cmd, "login") == 0) return cmdLogin(argc, argv);
    if (strcmp(cmd, "create_ticket") == 0) return cmdCreateTicket(argc, argv);
    if (strcmp(cmd, "close_ticket") == 0) return cmdCloseTicket(argc, argv);
    if (strcmp(cmd, "assign_ticket") == 0) return cmdAssignTicket(argc, argv);
    if (strcmp(cmd, "set_priority") == 0) return cmdSetPriority(argc, argv);
    if (strcmp(cmd, "set_notes") == 0) return cmdSetNotes(argc, argv);
    if (strcmp(cmd, "edit_ticket") == 0) return cmdEditTicket(argc, argv);
    if (strcmp(cmd, "delete_ticket") == 0) return cmdDeleteTicket(argc, argv);
    if (strcmp(cmd, "auto_assign_ticket") == 0) return cmdAutoAssignTicket(argc, argv);
    if (strcmp(cmd, "list_tickets") == 0) return cmdListTickets(argc, argv);
    if (strcmp(cmd, "search_bst") == 0) return cmdSearchBST(argc, argv);
    if (strcmp(cmd, "list_tickets_admin") == 0) return cmdListTicketsAdmin();
    if (strcmp(cmd, "list_engineers") == 0) return cmdListEngineers();

    printf("{\"success\":false,\"error\":\"Unknown command\"}\n");
    return 1;
}
