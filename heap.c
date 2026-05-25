#include "heap.h"

#include <stdio.h>

#include "json_output.h"

void insertMinHeap(Ticket** heap, int* size, Ticket* t) {
    int i = *size;
    (*size)++;
    heap[i] = t;
    while (i != 0 && heap[(i - 1) / 2]->priority > heap[i]->priority) {
        Ticket* temp = heap[i];
        heap[i] = heap[(i - 1) / 2];
        heap[(i - 1) / 2] = temp;
        i = (i - 1) / 2;
    }
}

Ticket* extractMin(Ticket** heap, int* size) {
    if (*size <= 0) return NULL;
    if (*size == 1) {
        (*size)--;
        return heap[0];
    }

    Ticket* root = heap[0];
    heap[0] = heap[*size - 1];
    (*size)--;

    int i = 0;
    while (1) {
        int smallest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < *size && heap[left]->priority < heap[smallest]->priority)
            smallest = left;
        if (right < *size && heap[right]->priority < heap[smallest]->priority)
            smallest = right;

        if (smallest != i) {
            Ticket* temp = heap[i];
            heap[i] = heap[smallest];
            heap[smallest] = temp;
            i = smallest;
        } else {
            break;
        }
    }

    return root;
}

void collectTicketsForHeap(TicketNode* root, Ticket** heap, int* size) {
    if (root == NULL) return;
    collectTicketsForHeap(root->leftChild, heap, size);
    if (root->data.priority > 0 && root->data.priority <= 5) {
        insertMinHeap(heap, size, &(root->data));
    }
    collectTicketsForHeap(root->rightChild, heap, size);
}

void printUnassignedTickets(TicketNode* root, int* isFirstElement) {
    if (root == NULL) return;
    printUnassignedTickets(root->leftChild, isFirstElement);
    if (root->data.priority <= 0 || root->data.priority > 5) {
        if (!(*isFirstElement)) printf(",");
        printTicketJSON(&(root->data));
        *isFirstElement = 0;
    }
    printUnassignedTickets(root->rightChild, isFirstElement);
}
