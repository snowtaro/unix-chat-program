#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <pthread.h>
#include <ncurses.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <wchar.h>
#include "chat_common.h"

#define USER_COLORS 5

static shm_t  *shm_ptr;
static sem_t  *mutex;
static int     client_index = -1;
static int     my_color;
static pid_t   my_pid;

static WINDOW *chat_win;
static WINDOW *input_win;
static WINDOW *list_win;
static int     max_y, max_x;
static int     last_msg = 0;
static char    nickname[32];

void cleanup();
void sigint_handler(int signum) {
    (void)signum;
    cleanup();
    exit(0);
}

void get_time_str(char *buf, size_t len, time_t t) {
    struct tm *tm_info = localtime(&t);
    strftime(buf, len, "%H:%M", tm_info);
}

void *message_listener(void *arg) {
    (void)arg;

    while (1) {
        sem_wait(mutex);
        int count = shm_ptr->msg_count;

        while (last_msg < count) {
            message_t *msg = &shm_ptr->messages[last_msg % MAX_MESSAGES];

            if (msg->dest_pid == 0 ||
                msg->dest_pid == my_pid ||
                msg->sender_pid == my_pid) {

                wattron(chat_win, COLOR_PAIR(msg->color_pair));
                char timebuf[16];
                get_time_str(timebuf, sizeof(timebuf), msg->timestamp);

                // 시간과 닉네임 출력
                wprintw(chat_win, "[%s] %s: ",
                        timebuf, msg->nickname);

                // UTF-8 → wide char 변환 후 메시지 출력
                {
                    wchar_t wbuf[MAX_MSG_LENGTH];
                    mbstowcs(wbuf, msg->text, MAX_MSG_LENGTH);
                    waddwstr(chat_win, wbuf);
                }
                // waddch(chat_win, '\n');
		int curry,currx;
		getyx(chat_win,curry,currx);
		(void)currx;
		wmove(chat_win,curry+1,1);

                wattroff(chat_win, COLOR_PAIR(msg->color_pair));
                wrefresh(chat_win);
            }
            last_msg++;
        }

        // 접속자 목록 업데이트
        werase(list_win);
        box(list_win, 0, 0);
        mvwprintw(list_win, 1, 1, "Users:");
        int line = 2;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (shm_ptr->clients[i].active) {
                wattron(list_win,
                        COLOR_PAIR(shm_ptr->clients[i].color_pair));
                mvwprintw(list_win, line, 1,
                          "%s", shm_ptr->clients[i].nickname);
                wattroff(list_win,
                         COLOR_PAIR(shm_ptr->clients[i].color_pair));
                line++;
            }
        }
        wrefresh(list_win);

        sem_post(mutex);
        usleep(100000);
    }
    return NULL;
}

void cleanup() {
    sem_wait(mutex);
    if (client_index >= 0) {
        shm_ptr->clients[client_index].active = 0;
    }
    sem_post(mutex);

    munmap(shm_ptr, sizeof(shm_t));
    sem_close(mutex);
    endwin();
}

int main() {
    setlocale(LC_ALL, "");
    my_pid = getpid();
    signal(SIGINT, sigint_handler);

    // Shared Memory 연결
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (shm_fd == -1) {
        perror("shm_open"); exit(1);
    }
    shm_ptr = mmap(NULL, sizeof(shm_t),
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap"); exit(1);
    }

    // Semaphore 열기
    mutex = sem_open(SEM_MUTEX_NAME, 0);
    if (mutex == SEM_FAILED) {
        perror("sem_open"); exit(1);
    }

    // 닉네임 입력
    printf("Enter your nickname: ");
    fgets(nickname, sizeof(nickname), stdin);
    nickname[strcspn(nickname, "\n")] = '\0';

    // 클라이언트 등록
    sem_wait(mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!shm_ptr->clients[i].active) {
            client_index = i;
            shm_ptr->clients[i].pid        = my_pid;
            strncpy(shm_ptr->clients[i].nickname,
                    nickname, 31);
            shm_ptr->clients[i].active     = 1;
            shm_ptr->clients[i].color_pair = (i % USER_COLORS) + 1;
            my_color = shm_ptr->clients[i].color_pair;
            break;
        }
    }
    sem_post(mutex);

    if (client_index == -1) {
        fprintf(stderr, "Server full. Try later.\n");
        exit(1);
    }

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    if (has_colors()) start_color();
    use_default_colors();
    init_pair(1, COLOR_RED,    -1);
    init_pair(2, COLOR_GREEN,  -1);
    init_pair(3, COLOR_YELLOW, -1);
    init_pair(4, COLOR_BLUE,   -1);
    init_pair(5, COLOR_MAGENTA,-1);

    getmaxyx(stdscr, max_y, max_x);
    int list_w = 20;

    chat_win  = newwin(max_y - 3, max_x - list_w, 0, 0);
    list_win  = newwin(max_y - 3, list_w,       0, max_x - list_w);
    input_win = newwin(3,       max_x, max_y - 3, 0);

    scrollok(chat_win, TRUE);
    box(chat_win,  0, 0);
    box(list_win,  0, 0);
    box(input_win, 0, 0);

    wrefresh(chat_win);
    wrefresh(list_win);
    wrefresh(input_win);
    wmove(chat_win,1,1);

    // 메시지 리스너 스레드
    pthread_t tid;
    pthread_create(&tid, NULL, message_listener, NULL);

    // 입력 루프
    char input[MAX_MSG_LENGTH];
    while (1) {
        werase(input_win);
        box(input_win, 0, 0);
        mvwprintw(input_win, 1, 1, "> ");
        wrefresh(input_win);

        echo();  // 입력 문자가 보이도록 echo 켬
        wgetnstr(input_win, input, MAX_MSG_LENGTH - 1);
        noecho();

        pid_t dest = 0;
        char  msg_text[MAX_MSG_LENGTH];

        // 귓속말 명령어 처리
        if (strncmp(input, "/whisper ", 9) == 0) {
            char dest_nick[32];
            char *space = strchr(input + 9, ' ');
            if (space) {
                int len = space - (input + 9);
                strncpy(dest_nick, input + 9, len);
                dest_nick[len] = '\0';
                strcpy(msg_text, space + 1);

                sem_wait(mutex);
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (shm_ptr->clients[i].active &&
                        strcmp(shm_ptr->clients[i].nickname,
                               dest_nick) == 0) {
                        dest = shm_ptr->clients[i].pid;
                        break;
                    }
                }
                sem_post(mutex);

                if (dest == 0) continue;
            }
        } else {
            strcpy(msg_text, input);
        }

        // 메시지 저장
        sem_wait(mutex);
        int idx = shm_ptr->msg_count % MAX_MESSAGES;
        message_t *msg = &shm_ptr->messages[idx];

        msg->sender_pid = my_pid;
        msg->dest_pid   = dest;
        strncpy(msg->nickname, nickname, 31);
        msg->color_pair = my_color;
        strncpy(msg->text, msg_text, MAX_MSG_LENGTH - 1);
        msg->timestamp  = time(NULL);

        shm_ptr->msg_count++;
        sem_post(mutex);
    }

    cleanup();
    return 0;
}
