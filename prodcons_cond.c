/* prodcons_cond.c — Producer-Consumer з умовними змінними */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUF_SIZE    8
#define NUM_ITEMS   30

static int buffer[BUF_SIZE];
static int in_idx  = 0;
static int out_idx = 0;
static int count   = 0;

static pthread_mutex_t mtx        = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond_full  = PTHREAD_COND_INITIALIZER;  /* сигнал: є дані */
static pthread_cond_t  cond_empty = PTHREAD_COND_INITIALIZER;  /* сигнал: є місце */

void *producer(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < NUM_ITEMS; i++) {
        int item = id * 100 + i;

        pthread_mutex_lock(&mtx);

        while (count >= BUF_SIZE) {
            pthread_cond_wait(&cond_empty, &mtx);
        }

        buffer[in_idx] = item;
        in_idx = (in_idx + 1) % BUF_SIZE;
        count++;

        printf("  Producer %d: [%d] → буфер (count=%d)\n", id, item, count);

        /* Сигнал споживачу, що з'явилися дані */
        pthread_cond_signal(&cond_full);

        pthread_mutex_unlock(&mtx);
    }

    return NULL;
}

void *consumer(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < NUM_ITEMS; i++) {
        pthread_mutex_lock(&mtx);

        /* Очікування, поки буфер порожній */
        while (count <= 0) {
            pthread_cond_wait(&cond_full, &mtx);
        }

        int item = buffer[out_idx];
        out_idx = (out_idx + 1) % BUF_SIZE;
        count--;

        printf("  Consumer %d: [%d] ← буфер (count=%d)\n", id, item, count);

        /* Сигнал виробнику, що з'явилося місце */
        pthread_cond_signal(&cond_empty);

        pthread_mutex_unlock(&mtx);
    }

    return NULL;
}

int main(void)
{
    int prod_id = 1, cons_id = 1;
    pthread_t prod_thread, cons_thread;

    printf("=== Producer-Consumer (буфер: %d, елементів: %d) ===\n",
           BUF_SIZE, NUM_ITEMS);

    pthread_create(&prod_thread, NULL, producer, &prod_id);
    pthread_create(&cons_thread, NULL, consumer, &cons_id);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cond_full);
    pthread_cond_destroy(&cond_empty);

    printf("=== Завершено ===\n");
    return EXIT_SUCCESS;
}
