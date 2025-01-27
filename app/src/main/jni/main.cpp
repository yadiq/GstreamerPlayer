//
// Created by Adi on 2024/12/30.
// Gstreamer

#include <jni.h>
#include <gst/gst.h>

JNIEXPORT jstring JNICALL
getVersion(JNIEnv *env, jclass thiz) {
    char *version_utf8 = gst_version_string ();
    jstring version_jstring = env->NewStringUTF (version_utf8);
    g_free (version_utf8);
    return version_jstring;
}

//构建 JNINativeMethod 数组
static JNINativeMethod methods[] = {
        {"getVersion", "()Ljava/lang/String;", (void *) getVersion},
};

//动态注册的入口函数
jint
JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    //获取JNI env变量
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    //获取native方法所在类
    jclass clazz = env->FindClass("org/freedesktop/gstreamer/GStreamerPlayer");
    if (clazz == NULL) {
        return -1;
    }
    // 动态注册native方法
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        return -1;
    }
    return JNI_VERSION_1_4;
}