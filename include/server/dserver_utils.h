#ifndef D_SERVER_UTILS_H_
#define D_SERVER_UTILS_H_

#define QUEUE_SIZE 10

typedef struct {
    void* data[QUEUE_SIZE];
    int front, rear, count;
} MessageQueue;

void enqueue(MessageQueue* queue, void* data);

void* dequeue(MessageQueue *q);

#endif