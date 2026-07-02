#ifndef KEEPALIVE_H
#define KEEPALIVE_H

#include <jni.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_yourpackage_keepalive_KeepAlive_startDaemon(JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_yourpackage_keepalive_KeepAlive_antiFreeze(JNIEnv* env, jobject thiz);

#ifdef __cplusplus
}
#endif

#endif
