#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct{
    int medicine_type;
    bool liquid_level_correct;
    bool liquid_color_correct;
    bool medicine_sealed_correct;
    bool label_correct;
    bool expiry_date_correct;
    bool pill_color_size_correct;
    bool pill_count_correct;
    bool pill_expiry_date_correct;

}UnprocessedMedicine;

typedef struct MedicineNode {
    UnprocessedMedicine data;
    struct MedicineNode* next;
} MedicineNode;

typedef struct {
    MedicineNode* front;
    MedicineNode* rear;
    int size;
} MedicineQueue;

void initializeMedicineQueue(MedicineQueue* queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

bool isMedicineQueueEmpty(MedicineQueue* queue) {
    return queue->size == 0;
}

void enqueueMedicine(MedicineQueue* queue, UnprocessedMedicine medicine) {
    MedicineNode* newNode = (MedicineNode*)malloc(sizeof(MedicineNode));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->data = medicine;
    newNode->next = NULL;

    if (isMedicineQueueEmpty(queue)) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
    queue->size++;
}

UnprocessedMedicine dequeueMedicine(MedicineQueue* queue) {
    if (isMedicineQueueEmpty(queue)) {
        fprintf(stderr, "Queue underflow\n");
        exit(EXIT_FAILURE);
    }

    MedicineNode* tempNode = queue->front;
    UnprocessedMedicine removedMedicine = tempNode->data;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    free(tempNode);
    queue->size--;
    return removedMedicine;
}

void makeMedicineQueueEmpty(MedicineQueue* queue) {
    while (!isMedicineQueueEmpty(queue)) {
        dequeueMedicine(queue);
    }
}
