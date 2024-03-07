#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

char* get_tty_for_pid(const char* pid);
int is_user_process(const char* pid, const char* tty);
unsigned long calculate_time(const char* pid);

int main() {
    printf("  PID TTY          TIME CMD\n");

    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("opendir");
        exit(1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (isdigit(entry->d_name[0])) {
            char pid[256];
            snprintf(pid, sizeof(pid), "%s", entry->d_name);

            char* tty = get_tty_for_pid(pid);
            if (tty && is_user_process(pid, tty)) {
                char path[256];
                snprintf(path, sizeof(path), "/proc/%s/stat", pid);
                FILE *statFile = fopen(path, "r");
                if (statFile) {
                    char cmd[256];
                    if (fscanf(statFile, "%*d (%[^)]) %*c", cmd) == 1) {
                        unsigned long time = calculate_time(pid);
                        printf("%5s %8s %02lu:%02lu:%02lu %s\n", pid, tty, time / 3600, (time % 3600) / 60, time % 60, cmd);
                    }
                    fclose(statFile);
                }
                free(tty);
            }
        }
    }

    closedir(dir);
    return 0;
}

// PID에 해당하는 프로세스의 TTY 값을 가져오는 함수
char* get_tty_for_pid(const char* pid) {
    char link_path[256];
    snprintf(link_path, sizeof(link_path), "/proc/%s/fd/0", pid);

    char tty[256];
    ssize_t len = readlink(link_path, tty, sizeof(tty));
    if (len != -1) {
        tty[len] = '\0';
        char* ptr = strstr(tty, "/dev/");
        if (ptr) {
            return strdup(ptr + 5);
        }
    }
    return NULL;
}

// 프로세스가 사용자 프로세스인지 확인하는 함수
int is_user_process(const char* pid, const char* tty) {
    if (strcmp(tty, "pts/0") == 0) { 
        return 1; // 사용자 프로세스
    }
    return 0; // 시스템 프로세스
}

// 프로세스의 시간을 계산하는 함수
unsigned long calculate_time(const char* pid) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    FILE *statFile = fopen(path, "r");
    if (statFile) {
        unsigned long utime, stime, cutime, cstime;
        if (fscanf(statFile, "%*d %*s %*s %*d %*d %*d %*d %*d %*u %*u %*u %*u %lu %lu %ld %ld", &utime, &stime, &cutime, &cstime) == 4) {
            fclose(statFile);
            long hertz = sysconf(_SC_CLK_TCK);
            unsigned long total_time = utime + stime + cutime + cstime;
            return total_time / hertz;
        }
        fclose(statFile);
    }
    return 0;
}

