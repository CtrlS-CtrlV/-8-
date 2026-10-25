/* thread_safe_queue.c — Потокобезпечна FIFO-черга */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define QUEUE_CAPACITY 16
#define NUM_PRODUCERS   2
#define NUM_CONSUMERS   2
#define ITEMS_PER_PRODUCER 10

typedef struct {
    int  data[QUEUE_CAPACITY];
    int  head;
    int  tail;
    int  count;
    pthread_mutex_t lock;
} ts_queue_t;

void queue_init(ts_queue_t *q)
{
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    pthread_mutex_init(&q->lock, NULL);
}

bool queue_enqueue(ts_queue_t *q, int value)
{
    pthread_mutex_lock(&q->lock);

    if (q->count >= QUEUE_CAPACITY) {
        pthread_mutex_unlock(&q->lock);
        return false;  /* черга повна */
    }
    
    q->data[q->tail] = value;
    q->tail = (q->tail + 1) % QUEUE_CAPACITY;
    q->count++;

    pthread_mutex_unlock(&q->lock);
    return true;
}

bool queue_dequeue(ts_queue_t *q, int *value)
{
    pthread_mutex_lock(&q->lock);

    if (q->count <= 0) {
        pthread_mutex_unlock(&q->lock);
        return false;  /* черга порожня */
    }

    /* Вилучення елемента з черги */
    *value = q->data[q->head];
    q->head = (q->head + 1) % QUEUE_CAPACITY;
    q->count--;

    pthread_mutex_unlock(&q->lock);
    return true;
}

void queue_destroy(ts_queue_t *q)
{
    pthread_mutex_destroy(&q->lock);
}

static ts_queue_t queue;

void *producer_func(void *arg)
{
    int id = *(int *)arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = id * 1000 + i;
        while (!queue_enqueue(&queue, item)) {
            usleep(1000);  /* повторна спроба через 1 мс */
        }
        printf("  Producer %d: додав %d\n", id, item);
    }
    return NULL;
}

void *consumer_func(void *arg)
{
    int id = *(int *)arg;
    int total_consumed = 0;
    int items_to_consume = (NUM_PRODUCERS * ITEMS_PER_PRODUCER) / NUM_CONSUMERS;

    for (int i = 0; i < items_to_consume; i++) {
        int item;
        while (!queue_dequeue(&queue, &item)) {
            usleep(1000);
        }
        printf("  Consumer %d: забрав %d\n", id, item);
        total_consumed++;
    }

    printf("  Consumer %d: всього оброблено %d елементів\n",
           id, total_consumed);
    return NULL;
}

int main(void)
{
    queue_init(&queue);

    pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMERS];
    int prod_ids[NUM_PRODUCERS], cons_ids[NUM_CONSUMERS];

    printf("=== Thread-Safe Queue (producers=%d, consumers=%d) ===\n",
           NUM_PRODUCERS, NUM_CONSUMERS);

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        prod_ids[i] = i + 1;
        pthread_create(&producers[i], NULL, producer_func, &prod_ids[i]);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        cons_ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer_func, &cons_ids[i]);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++)
        pthread_join(producers[i], NULL);
    for (int i = 0; i < NUM_CONSUMERS; i++)
        pthread_join(consumers[i], NULL);

    queue_destroy(&queue);
    printf("=== Завершено ===\n");

    return EXIT_SUCCESS;
}
