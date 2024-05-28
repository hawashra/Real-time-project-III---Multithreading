#ifndef INCLUDE_H
#include "include.h"
#endif

// Define the node structure
typedef struct Node_pthread {
    pthread_t thread_id;
    struct Node* next;
} Node;

// Function to create a new node
Node* create_node_pthread(pthread_t thread_id) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    new_node->thread_id = thread_id;
    new_node->next = NULL;
    return new_node;
}

// Function to insert a node at the end of the list
void insert_node_pthread(Node** head, pthread_t thread_id) {
    Node* new_node = create_node(thread_id);
    if (*head == NULL) {
        *head = new_node;
    } else {
        Node* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
}

// Function to print the linked list
void print_list_pthread(Node* head) {
    Node* temp = head;
    while (temp != NULL) {
        printf("%lu -> ", temp->thread_id);
        temp = temp->next;
    }
    printf("NULL\n");
}

// Function to free the linked list
void free_list_pthread(Node* head) {
    Node* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}