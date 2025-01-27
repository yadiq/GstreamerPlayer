LOCAL_PATH := $(call my-dir) #设置当前模块的编译路径为当前文件夹路径

include $(CLEAR_VARS) #清理(可能由其他模块设置过的)编译环境中用到的变量

LOCAL_MODULE    := gstreamer_player #当前模块的名称
LOCAL_SRC_FILES := main.cpp dummy.cpp #当前模块包含的源代码文件
LOCAL_SHARED_LIBRARIES := gstreamer_android #当前模块在运行时依赖的动态库
LOCAL_LDLIBS := -llog -landroid #连接系统库
include $(BUILD_SHARED_LIBRARY) #编译共享库

ifndef GSTREAMER_ROOT_ANDROID
$(error GSTREAMER_ROOT_ANDROID is not defined!)
endif

ifeq ($(TARGET_ARCH_ABI),armeabi)
GSTREAMER_ROOT        := $(GSTREAMER_ROOT_ANDROID)/arm
else ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
GSTREAMER_ROOT        := $(GSTREAMER_ROOT_ANDROID)/armv7
else ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
GSTREAMER_ROOT        := $(GSTREAMER_ROOT_ANDROID)/arm64
else ifeq ($(TARGET_ARCH_ABI),x86)
GSTREAMER_ROOT        := $(GSTREAMER_ROOT_ANDROID)/x86
else ifeq ($(TARGET_ARCH_ABI),x86_64)
GSTREAMER_ROOT        := $(GSTREAMER_ROOT_ANDROID)/x86_64
else
$(error Target arch ABI not supported: $(TARGET_ARCH_ABI))
endif

GSTREAMER_NDK_BUILD_PATH  := $(GSTREAMER_ROOT)/share/gst-android/ndk-build/
include $(GSTREAMER_NDK_BUILD_PATH)/plugins.mk

# 插件说明
# GSTREAMER_PLUGINS_CORE：videoconvert 软解视频格式转换（不能删除no element "autovideosink"）
# GSTREAMER_PLUGINS_CODECS：androidmedia 硬件解码，访问设备可用编解码器（不能删除 黑屏）（官方的问题运行时报错GStreamer.init(context)）
# GSTREAMER_PLUGINS_SYS：opensles 音频硬件加速；opengl 图形硬件加速（不能删除 eglQuerySurface）
# GSTREAMER_PLUGINS_NET：rtp udp 网络（不能删除no element "udpsrc"）
# GSTREAMER_PLUGINS_PLAYBACK：playback（不能删除no element "parsebin"）
# GSTREAMER_PLUGINS_EFFECTS：视频效果
GSTREAMER_PLUGINS         := $(GSTREAMER_PLUGINS_CORE) $(GSTREAMER_PLUGINS_SYS) $(GSTREAMER_PLUGINS_EFFECTS)

# gobject-2.0不再使用 音频无需使用
GSTREAMER_EXTRA_DEPS      := gstreamer-video-1.0 gobject-2.0
# 字符编码
GSTREAMER_EXTRA_LIBS      := -liconv

include $(GSTREAMER_NDK_BUILD_PATH)/gstreamer-1.0.mk
