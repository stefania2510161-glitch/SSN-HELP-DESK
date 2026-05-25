#include "bst.h"

#include <stdlib.h>

#include "helpdesk_state.h"
#include "json_output.h"

TicketNode* createNewTicketNode(Ticket newTicket) {
    TicketNode* newNode = (TicketNode*)malloc(sizeof(TicketNode));
    if (newNode != NULL) {
        newNode->data = newTicket;
        newNode->leftChild = NULL;
        newNode->rightChild = NULL;
    }
    return newNode;
}

void freeBST(TicketNode* root) {
    if (root == NULL) return;
    freeBST(root->leftChild);
    freeBST(root->rightChild);
    free(root);
}

TicketNode* insertTicketIntoBST(TicketNode* root, Ticket newTicket) {
    if (root == NULL) return createNewTicketNode(newTicket);
    if (newTicket.id < root->data.id) root->leftChild = insertTicketIntoBST(root->leftChild, newTicket);
    else if (newTicket.id > root->data.id) root->rightChild = insertTicketIntoBST(root->rightChild, newTicket);
    return root;
}

static TicketNode* findMin(TicketNode* node) {
    while (node->leftChild != NULL) node = node->leftChild;
    return node;
}

TicketNode* removeTicketFromBST(TicketNode* root, int targetId) {
    if (root == NULL) return NULL;
    if (targetId < root->data.id) {
        root->leftChild = removeTicketFromBST(root->leftChild, targetId);
    } else if (targetId > root->data.id) {
        root->rightChild = removeTicketFromBST(root->rightChild, targetId);
    } else {
        if (root->leftChild == NULL) {
            TicketNode* temp = root->rightChild;
            free(root);
            return temp;
        } else if (root->rightChild == NULL) {
            TicketNode* temp = root->leftChild;
            free(root);
            return temp;
        }
        TicketNode* temp = findMin(root->rightChild);
        root->data = temp->data;
        root->rightChild = removeTicketFromBST(root->rightChild, temp->data.id);
    }
    return root;
}

TicketNode* searchTicketInBST(TicketNode* root, int targetId) {
    if (root == NULL || root->data.id == targetId) return root;
    if (targetId < root->data.id) return searchTicketInBST(root->leftChild, targetId);
    return searchTicketInBST(root->rightChild, targetId);
}

void writeTicketsToDiskRecursive(TicketNode* root, FILE* file) {
    if (root == NULL) return;
    writeTicketsToDiskRecursive(root->leftChild, file);
    fwrite(&(root->data), sizeof(Ticket), 1, file);
    writeTicketsToDiskRecursive(root->rightChild, file);
}

void printTicketsInOrder(TicketNode* root, int targetUid, int* isFirstElement) {
    if (root == NULL) return;
    printTicketsInOrder(root->leftChild, targetUid, isFirstElement);
    if (targetUid == 0 || root->data.uid == targetUid) {
        if (!(*isFirstElement)) printf(",");
        printTicketJSON(&(root->data));
        *isFirstElement = 0;
    }
    printTicketsInOrder(root->rightChild, targetUid, isFirstElement);
}
