#include "KeepAlive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <android/log.h>
#include <sys/prctl.h>
#include <linux/oom.h>

#define TAG "KeepAlive"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static pid_t guardian_pid = 0;
static pid_t app_pid = 0;

void set_process_critical() {
    int fd = open("/proc/self/oom_score_adj", O_WRONLY);
    if (fd >= 0) {
        char buf[16];
        sprintf(buf, "%d", -950);
        write(fd, buf, strlen(buf));
        close(fd);
    }
    prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, 0, 0, "keepalive");
}

JNIEXPORT void JNICALL Java_com_yourpackage_keepalive_KeepAlive_startDaemon(JNIEnv* env, jobject thiz) {
    set_process_critical();
    app_pid = getpid();
    signal(SIGCHLD, SIG_IGN);
    guardian_pid = fork();

    if (guardian_pid == 0) {
        nice(-20);
        set_process_critical();
        while(1) {
            if (kill(app_pid, 0) == -1) {
                LOGE("主进程死亡，启动复活流程");
                system("am start -n com.yourpackage.keepalive/.MainActivity --activity-new-task");
                sleep(2);
                FILE* fp = popen("pidof com.yourpackage.keepalive", "r");
                if (fp) {
                    fscanf(fp, "%d", &app_pid);
                    pclose(fp);
                }
            }
            sleep(2);
        }
    } else {
        LOGI("守护进程启动，PID: %d", guardian_pid);
        while(1) {
            if (kill(guardian_pid, 0) == -1) {
                LOGE("守护进程死亡，重启");
                guardian_pid = fork();
                if (guardian_pid == 0) {
                    continue;
                }
            }
            sleep(5);
        }
    }
}

JNIEXPORT void JNICALL Java_com_yourpackage_keepalive_KeepAlive_showPopup(JNIEnv* env, jobject thiz) {
    system("settings put global oneplus_game_mode_whitelist com.yourpackage.keepalive");
    char cmd[512];
    sprintf(cmd, "am start -n com.yourpackage.keepalive/.PopupActivity --activity-clear-task --activity-new-task -f 0x10000000");
    system(cmd);
}

JNIEXPORT void JNICALL Java_com_yourpackage_keepalive_KeepAlive_antiFreeze(JNIEnv* env, jobject thiz) {
    int epfd = epoll_create1(0);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    int pipe_fd[2];
    pipe(pipe_fd);
    epoll_ctl(epfd, EPOLL_CTL_ADD, pipe_fd[0], &ev);

    while(1) {
        epoll_wait(epfd, &ev, 1, 1000);
        asm volatile("" ::: "memory");
    }
}

JNIEXPORT void JNICALL Java_com_yourpackage_keepalive_KeepAlive_installAutoStart(JNIEnv* env, jobject thiz) {
    system("am start -n com.yourpackage.keepalive/.MainActivity --activity-new-task");
}
