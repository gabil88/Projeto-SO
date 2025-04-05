#ifndef D_SERVER_UTILS_H_
#define D_SERVER_UTILS_H_

#define QUEUE_SIZE 10

typedef struct {
    char messages[QUEUE_SIZE][256];
    int front;
    int rear;
    int count;
} MessageQueue;

void enqueue(MessageQueue *q, const char *message);



#endif