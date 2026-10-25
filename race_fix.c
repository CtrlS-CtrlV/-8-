/* race_fix.c — Демонстрація гонки даних та виправлення */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS   4
#define INCREMENTS    1000000

static long counter_unsafe = 0;

static long counter_safe = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void *increment_unsafe(void *arg)
{
    (void)arg;
    for (int i = 0; i < INCREMENTS; i++) {
        counter_unsafe++;  /* read-modify-write — НЕ атомарно! */
    }
    return NULL;
}

void *increment_safe(void *arg)
{
    (void)arg;
    for (int i = 0; i < INCREMENTS; i++) {
        pthread_mutex_lock(&mtx);
        counter_safe++;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_THREADS];
    long expected = (long)NUM_THREADS * INCREMENTS;

    /* === Тест 1: без синхронізації === */
    printf("=== Без синхронізації ===\n");
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, increment_unsafe, NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    printf("Очікувано: %ld\n", expected);
    printf("Отримано:  %ld %s\n\n", counter_unsafe,
           (counter_unsafe == expected) ? "(збіг)" : "(ГОНКА ДАНИХ!)");

    /* === Тест 2: з м'ютексом === */
    printf
