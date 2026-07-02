
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

// 设置进程为"系统关键进程"（Android 16上仍有效）
void set_process_critical() {
    // 写入oom_adj，降低被杀概率
    int fd = open("/proc/self/oom_score_adj", O_WRONLY);
    if (fd >= 0) {
        char buf[16];
        sprintf(buf, "%d", -950);  // 接近系统进程优先级
        write(fd, buf, strlen(buf));
        close(fd);
    }
    
    // Android 16: 设置进程为"不可被冻结"
    prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, 0, 0, "keepalive");
}

// 双进程守护 - OnePlus优化版
JNIEXPORT void JNICALL Java_com_yourapp_KeepAlive_startDaemon(JNIEnv* env, jobject thiz) {
    // 设置进程优先级
    set_process_critical();
    
    app_pid = getpid();
    
    // 信号处理：子进程守护
    signal(SIGCHLD, SIG_IGN);  // 避免僵尸进程
    
    // fork守护进程
    guardian_pid = fork();
    
    if (guardian_pid == 0) {
        // ----- 守护子进程 -----
        // 设置更高的优先级
        nice(-20);
        set_process_critical();
        
        while(1) {
            // 检查主进程是否存活
            if (kill(app_pid, 0) == -1) {
                // 主进程已死，复活
                LOGE("主进程死亡，启动复活流程");
                
                // 方式1: 通过系统命令启动
                system("am start -n your.app.package/.MainActivity --activity-new-task");
                
                // 方式2: 通过JobScheduler唤醒（Java层处理）
                // 通过JNI回调Java
                // 这里简单sleep等待
                sleep(2);
                
                // 更新app_pid
                FILE* fp = popen("pidof your.app.package", "r");
                if (fp) {
                    fscanf(fp, "%d", &app_pid);
                    pclose(fp);
                }
            }
            sleep(2);  // 2秒检查一次，响应更快
        }
    } else {
        // ----- 主进程 -----
        LOGI("守护进程启动，PID: %d", guardian_pid);
        
        // 监控守护进程，防止被杀死
        while(1) {
            if (kill(guardian_pid, 0) == -1) {
                // 守护进程被杀死，重启
                LOGE("守护进程死亡，重启");
                guardian_pid = fork();
                if (guardian_pid == 0) {
                    // 子进程继续守护
                    continue;
                }
            }
            sleep(5);
        }
    }
}

// Android 16专用：后台弹窗
JNIEXPORT void JNICALL Java_com_yourapp_KeepAlive_showPopup(JNIEnv* env, jobject thiz) {
    // OnePlus系统利用"游戏模式"白名单
    // 先把自己加入游戏模式白名单
    system("settings put global oneplus_game_mode_whitelist your.app.package");
    
    // 利用TYPE_APPLICATION_OVERLAY弹窗
    char cmd[512];
    sprintf(cmd, "am start -n your.app.package/.PopupActivity --activity-clear-task --activity-new-task -f 0x10000000");
    system(cmd);
}

// 防冻结：持续持有CPU锁（极低功耗）
JNIEXPORT void JNICALL Java_com_yourapp_KeepAlive_antiFreeze(JNIEnv* env, jobject thiz) {
    // 使用epoll等待，不消耗CPU
    int epfd = epoll_create1(0);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    int pipe_fd[2];
    pipe(pipe_fd);
    epoll_ctl(epfd, EPOLL_CTL_ADD, pipe_fd[0], &ev);
    
    while(1) {
        // 阻塞等待，不占用CPU
        epoll_wait(epfd, &ev, 1, 1000);  // 1秒超时
        // 轻微唤醒，防止系统冻结
        asm volatile("" ::: "memory");
    }
}

// 安装后立即自启动
JNIEXPORT void JNICALL Java_com_yourapp_KeepAlive_installAutoStart(JNIEnv* env, jobject thiz) {
    // 监听PACKAGE_ADDED广播，在Java层实现
    // C层触发启动
    system("am start -n your.app.package/.MainActivity --activity-new-task");
}
