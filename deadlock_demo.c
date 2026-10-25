/* deadlock_demo.c — Демонстрація та виправлення дедлоку */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

static pthread_mutex_t mutex_A = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_B = PTHREAD_MUTEX_INITIALIZER;

/* === ВЕРСІЯ 1: ДЕДЛОК (НЕБЕЗПЕЧНА) === */

void *thread1_deadlock(void *arg)
{
    (void)arg;
    printf("  [T1] Захоплюю mutex_A...\n");
    pthread_mutex_lock(&mutex_A);
    printf("  [T1] Отримав mutex_A. Затримка...\n");
    usleep(100000);

    printf("  [T1] Захоплюю mutex_B...\n");
    pthread_mutex_lock(&mutex_B);
    printf("  [T1] Отримав mutex_B. Критична секція.\n");

    pthread_mutex_unlock(&mutex_B);
    pthread_mutex_unlock(&mutex_A);
    return NULL;
}

void *thread2_deadlock(void *arg)
{
    (void)arg;
    printf("  [T2] Захоплюю mutex_B...\n");
    pthread_mutex_lock(&mutex_B);
    printf("  [T2] Отримав mutex_B. Затримка...\n");
    usleep(100000);

    printf("  [T2] Захоплюю mutex_A...\n");
    pthread_mutex_lock(&mutex_A);
    printf("  [T2] Отримав mutex_A. Критична секція.\n");

    pthread_mutex_unlock(&mutex_A);
    pthread_mutex_unlock(&mutex_B);
    return NULL;
}

/* === ВЕРСІЯ 2: ВИПРАВЛЕННЯ ЧЕРЕЗ LOCK ORDERING === */

void *thread1_ordered(void *arg)
{
    (void)arg;
    printf("  [T1-fix] Захоплюю mutex_A → mutex_B (порядок)...\n");
    pthread_mutex_lock(&mutex_A);
    pthread_mutex_lock(&mutex_B);
    
    printf("  [T1-fix] Критична секція. Звільняю.\n");
    
    pthread_mutex_unlock(&mutex_B);
    pthread_mutex_unlock(&mutex_A);
    return NULL;
}

void *thread2_ordered(void *arg)
{
    (void)arg;
    printf("  [T2-fix] Захоплюю mutex_A → mutex_B (той самий порядок)...\n");
    pthread_mutex_lock(&mutex_A);
    pthread_mutex_lock(&mutex_B);
    
    printf("  [T2-fix] Критична секція. Звільняю.\n");
    
    pthread_mutex_unlock(&mutex_B);
    pthread_mutex_unlock(&mutex_A);
    return NULL;
}

/* === ВЕРСІЯ 3: ВИПРАВЛЕННЯ ЧЕРЕЗ trylock === */

void *thread_trylock(void *arg)
{
    int id = *(int *)arg;
    int acquired = 0;

    while (!acquired) {
        pthread_mutex_lock(&mutex_A);

        int rc = pthread_mutex_trylock(&mutex_B);
        if (rc == 0) {
            printf("  [T%d-trylock] Обидва м'ютекси захоплено!\n", id);
            acquired = 1;

            printf("  [T%d-trylock] Критична секція.\n", id);

            pthread_mutex_unlock(&mutex_B);
            pthread_mutex_unlock(&mutex_A);
        } else {
            pthread_mutex_unlock(&mutex_A);
            printf("  [T%d-trylock] trylock невдалий, відступаю...\n", id);
            usleep(10000 + rand() % 50000);  /* випадкова затримка */
        }
    }

    return NULL;
}

int main(void)
{
    pthread_t t1, t2;

    /* --- Тест 2: Lock ordering --- */
    printf("=== Тест 2: Lock Ordering (виправлення) ===\n");
    pthread_create(&t1, NULL, thread1_ordered, NULL);
    pthread_create(&t2, NULL, thread2_ordered, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    /* --- Тест 3: trylock --- */
    printf("\n=== Тест 3: trylock (виправлення) ===\n");
    int id1 = 1, id2 = 2;
    pthread_create(&t1, NULL, thread_trylock, &id1);
    pthread_create(&t2, NULL, thread_trylock, &id2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&mutex_A);
    pthread_mutex_destroy(&mutex_B);

    printf("\nВсі тести завершено.\n");
    return EXIT_SUCCESS;
}
