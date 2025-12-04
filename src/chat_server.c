#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "chat_common.h"

static shm_t *shm_ptr = NULL;
static sem_t *mutex   = NULL;

void cleanup(int signum) {
    (void) signum;

    if (mutex) sem_close(mutex), sem_unlink(SEM_MUTEX_NAME);
    if (shm_ptr) munmap(shm_ptr, sizeof(shm_t)), shm_unlink(SHM_NAME);
    printf("\nServer shutting down. Cleaned up shared memory and semaphores.\n");
    exit(0);
}

int main() {
    signal(SIGINT, cleanup);

    // Shared Memory create and map
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open"); exit(1); }

    if (ftruncate(shm_fd, sizeof(shm_t)) == -1) {
        perror("ftruncate"); exit(1);
    }

    shm_ptr = mmap(NULL, sizeof(shm_t),
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap"); exit(1);
    }

    // Semaphore create
    sem_unlink(SEM_MUTEX_NAME);
    mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open"); exit(1);
    }

    // Server initialize
    sem_wait(mutex);
    memset(shm_ptr, 0, sizeof(shm_t));
    sem_post(mutex);

    printf("Chat server started. Shared memory and semaphore initialized.\n");
    printf("Press Ctrl+C to stop the server.\n");

    // Server waits for SIGINT
    while (1) {
        pause();
    }

    return 0;
}
