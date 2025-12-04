#ifndef CHAT_COMMON_H
#define CHAT_COMMON_H

#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX_CLIENTS    5
#define MAX_MSG_LENGTH 512
#define MAX_MESSAGES   100

#define SHM_NAME        "/chat_shm"
#define SEM_MUTEX_NAME  "/chat_mutex"

// Client information struct 
typedef struct {
    pid_t pid;
    char  nickname[32];
    int   active;
    int   color_pair;
} client_t;

// Message information struct 
typedef struct {
    pid_t    sender_pid;
    pid_t    dest_pid;       // 0:broadcast
    char     nickname[32];
    int      color_pair;
    char     text[MAX_MSG_LENGTH];
    time_t   timestamp;
} message_t;

// Shared Memory total layout 
typedef struct {
    client_t  clients[MAX_CLIENTS];
    message_t messages[MAX_MESSAGES];
    int       msg_count;
} shm_t;

#endif // CHAT_COMMON_H
