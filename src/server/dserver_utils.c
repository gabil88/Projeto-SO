#include <string.h>
#include <pthread.h>
#include "dserver_utils.h"

void enqueue(MessageQueue *q, const char *message) {
    if (q->count < QUEUE_SIZE) {
        strncpy(q->messages[q->rear], message, 256);
        q->rear = (q->rear + 1) % QUEUE_SIZE;
        q->count++;
    } else {
        printf("Queue is full. Message dropped: %s\n", message);
    }
}