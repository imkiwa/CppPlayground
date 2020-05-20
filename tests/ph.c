#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#include <semaphore.h>
#include <unistd.h>

static pthread_t threads[5];
static sem_t chop[5];
static sem_t room;

static void dropChop(int id) {
    fprintf(stderr, "[%d] drop chopstick\n", id);
    int left = (id + 5 - 1) % 5;
    int right = (id + 1) % 5;
    sem_post(&chop[left]);
    sem_post(&chop[right]);
}

static void takeForks(int id) {
    int left = (id + 5 - 1) % 5;
    int right = (id + 1) % 5;
    sem_wait(&chop[left]);
    sem_wait(&chop[right]);
}

static void haveMeal(int id) {
    sleep(3);
    fprintf(stderr, "[%d] have meal\n", id);
}

static void think(int id) {
    sleep(3);
    fprintf(stderr, "[%d] think\n", id);
}

static void *philosopherThread(void *arg) {
    int id = (int) arg;
    fprintf(stderr, "philosopher [%d] just born\n", id);

    while (true) {
        think(id);
        sem_wait(&room);
        takeForks(id);
        sem_post(&room);
        haveMeal(id);
        dropChop(id);
    }
}

int main() {
    for (int i = 0; i < 5; i++) {
        if (sem_init(&chop[i], 0, 1) != 0) {
            perror("sem_init");
        }
    }

    sem_init(&room, 0, 4);

    for (int i = 0; i < 5; ++i) {
        if (pthread_create(&threads[i], NULL, philosopherThread, (void *) i) != 0) {
            perror("pthread_create");
        }
    }

    while (true) {
        continue;
    }
}
