#include <string.h>
#include <stdio.h>

#include "../../include/server/dserver_utils.h"



// Em dserver_utils.c
void enqueue(MessageQueue* queue, void* data) {
    if (queue->count >= QUEUE_SIZE) {
        printf("Queue is full\n");
        return;
    }
    
    queue->data[queue->rear] = data;
    queue->rear = (queue->rear + 1) % QUEUE_SIZE;
    queue->count++;
}

void* dequeue(MessageQueue* queue) {
    if (queue->count <= 0) {
        printf("Queue is empty\n");
        return NULL;
    }
    
    void* data = queue->data[queue->front];
    queue->front = (queue->front + 1) % QUEUE_SIZE;
    queue->count--;
    
    return data;
}