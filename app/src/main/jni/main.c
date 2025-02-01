#include <string.h>
#include <stdint.h>
#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <gst/gst.h>
#include <pthread.h>
#include <gst/video/videooverlay.h>

GST_DEBUG_CATEGORY_STATIC (debug_category);
#define GST_CAT_DEFAULT debug_category

/*
 * These macros provide a way to store the native pointer to CustomData, which might be 32 or 64 bits, into
 * a jlong, which is always 64 bits, without warnings.
 */
#if GLIB_SIZEOF_VOID_P == 8
# define GET_CUSTOM_DATA(env, thiz, fieldID) (CustomData *)(*env)->GetLongField (env, thiz, fieldID)
# define SET_CUSTOM_DATA(env, thiz, fieldID, data) (*env)->SetLongField (env, thiz, fieldID, (jlong)data)
#else
# define GET_CUSTOM_DATA(env, thiz, fieldID) (CustomData *)(jint)(*env)->GetLongField (env, thiz, fieldID)
# define SET_CUSTOM_DATA(env, thiz, fieldID, data) (*env)->SetLongField (env, thiz, fieldID, (jlong)(jint)data)
#endif

#define  LOG_TAG    "JNI_log"

/* Structure to contain all our information, so we can pass it to callbacks */
//包含所有信息的结构，便于传递。占8个字节，java中用long表示
typedef struct _CustomData {
    jobject app;                  /* Application instance, used to call its methods. A global reference is kept. */ //全局 Jobject 变量, 用于存储jobject thiz
    char pipeline_description[1000] ;//管道描述
    GstElement *pipeline;         /* The running pipeline */
    GMainContext *context;        /* GLib context used to run the main loop *///GLib上下文
    GMainLoop *main_loop;         /* GLib main loop *///Glib主循环
    gboolean initialized;         /* To avoid informing the UI multiple times about the initialization */
    pthread_t gst_app_thread;//线程id
    GstElement *video_sink;       /* The video sink element which receives XOverlay commands */
    ANativeWindow *native_window; /* The Android native window where video will be rendered */
} CustomData;

/* These global variables cache values which are not changing during execution */
//全局静态变量。在多线程的环境下，进程内的所有线程共享进程的数据空间。因此全局变量为所有线程共享
//static pthread_t gst_app_thread;//线程id改为局部变量
static pthread_key_t current_jni_env;
//线程私有数据的键值。采用了一键多值的技术，即一个键对应多个值。
//一键多值:各线程可根据自己的需要往key中填入不同的值，这就相当于提供了一个同名而不同值的全局变量。
static JavaVM *java_vm;//多实例使用相同的java_vm env
static jfieldID custom_data_field_id;
static jmethodID set_message_method_id;
static jmethodID on_gstreamer_initialized_method_id;

/*
 * Private methods
 */

/* Register this thread with the VM */
//从全局的JavaVM中获取到环境变量
static JNIEnv *
attach_current_thread(void) {
    JNIEnv *env;
    JavaVMAttachArgs args;

    GST_DEBUG ("Attaching thread %p", g_thread_self());
    args.version = JNI_VERSION_1_4;
    args.name = NULL;
    args.group = NULL;

    if ((*java_vm)->AttachCurrentThread(java_vm, &env, &args) < 0) {
        GST_ERROR ("Failed to attach current thread");
        return NULL;
    }

    return env;
}

/* Unregister this thread from the VM */
static void
detach_current_thread(void *env) {
    GST_DEBUG ("Detaching thread %p", g_thread_self());
    (*java_vm)->DetachCurrentThread(java_vm);
}

/* Retrieve the JNI environment for this thread */
//从线程私有数据中获取到环境变量
static JNIEnv *
get_jni_env(void) {
    JNIEnv *env;

    if ((env = pthread_getspecific(current_jni_env)) == NULL) {
        env = attach_current_thread();
        pthread_setspecific(current_jni_env, env);
    }

    return env;
}

/* Change the content of the UI's TextView */
void
set_ui_message(const gchar *message, CustomData *data) {
    JNIEnv *env = get_jni_env();
    GST_DEBUG ("Setting message to: %s", message);
    jstring jmessage = (*env)->NewStringUTF(env, message);
    (*env)->CallVoidMethod(env, data->app, set_message_method_id, jmessage);
    if ((*env)->ExceptionCheck(env)) {
        GST_ERROR ("Failed to call Java method");
        (*env)->ExceptionClear(env);
    }
    (*env)->DeleteLocalRef(env, jmessage);
}

/* Retrieve errors from the bus and show them on the UI */
void
error_cb(GstBus *bus, GstMessage *msg, CustomData *data) {
    GError *err;
    gchar *debug_info;
    gchar *message_string;

    gst_message_parse_error(msg, &err, &debug_info);
    message_string =
            g_strdup_printf("Error received from element %s: %s",
                            GST_OBJECT_NAME (msg->src), err->message);
    g_clear_error(&err);
    g_free(debug_info);
    set_ui_message(message_string, data);
    g_free(message_string);
    gst_element_set_state(data->pipeline, GST_STATE_NULL);
}

/* Notify UI about pipeline state changes */
void
state_changed_cb(GstBus *bus, GstMessage *msg, CustomData *data) {
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
    /* Only pay attention to messages coming from the pipeline, not its children */
    if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->pipeline)) {
        gchar *message = g_strdup_printf("State changed to %s",
                                         gst_element_state_get_name(new_state));
        set_ui_message(message, data);
        g_free(message);
    }
}

/* Check if all conditions are met to report GStreamer as initialized.
 * These conditions will change depending on the application */
//初始化完成
void
check_initialization_complete(CustomData *data) {
    JNIEnv *env = get_jni_env();
    if (!data->initialized && data->native_window && data->main_loop) {
        GST_DEBUG ("Initialization complete, notifying application. native_window:%p main_loop:%p", data->native_window, data->main_loop);

        //为sink设置窗口句柄
        /* The main loop is running and we received a native window, inform the sink about it */
        gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (data->video_sink), (guintptr) data->native_window);

        (*env)->CallVoidMethod(env, data->app, on_gstreamer_initialized_method_id);
        if ((*env)->ExceptionCheck(env)) {
            GST_ERROR ("Failed to call Java method");
            (*env)->ExceptionClear(env);
        }

        //开始播放 TODO
        gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
        data->initialized = TRUE;
    }
}

/* Main method for the native code. This is executed on its own thread. */
//工作线程执行
void *
app_function(void *userdata) {
    JavaVMAttachArgs args;
    GstBus *bus;
    CustomData *data = (CustomData *) userdata;
    GSource *bus_source;
    GError *error = NULL;

    GST_DEBUG ("Creating pipeline in CustomData at %p", data);

    /* Create our own GLib Main Context and make it the default one */
    data->context = g_main_context_new();//创建GLib上下文
    g_main_context_push_thread_default(data->context);//将其设置为线程的默认上下文

    /* Build pipeline */
    //data->pipeline = gst_parse_launch ("videotestsrc ! warptv ! videoconvert ! autovideosink",&error);
    data->pipeline = gst_parse_launch (data -> pipeline_description,&error);

    //创建管道：产生数据 对频实时处理 视频格式转换 消费数据
    if (error) {
        gchar *message = g_strdup_printf("Unable to build pipeline: %s", error->message);
        g_clear_error(&error);
        set_ui_message(message, data);
        g_free(message);
        return NULL;
    }

    //设置管道状态ready, 接受窗口句柄
    /* Set the pipeline to READY, so it can already accept a window handle, if we have one */
    gst_element_set_state (data->pipeline, GST_STATE_READY);
    //获取视频槽，消费者
    data->video_sink = gst_bin_get_by_interface (GST_BIN (data->pipeline),GST_TYPE_VIDEO_OVERLAY);
    if (!data->video_sink) {
        GST_ERROR ("Could not retrieve video sink");
        return NULL;
    }

    /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
    //指示总线为每个收到的消息发出信号，并连接到感兴趣的信号
    bus = gst_element_get_bus(data->pipeline);//管道总线
    bus_source = gst_bus_create_watch(bus);
    g_source_set_callback(bus_source, (GSourceFunc) gst_bus_async_signal_func, NULL, NULL);
    g_source_attach(bus_source, data->context);
    g_source_unref(bus_source);
    g_signal_connect (G_OBJECT(bus), "message::error", (GCallback) error_cb, data);//异步地为GObject连接信号和回调
    g_signal_connect (G_OBJECT(bus), "message::state-changed", (GCallback) state_changed_cb, data);
    gst_object_unref(bus);

    /* Create a GLib Main Loop and set it to run */
    GST_DEBUG ("Entering main loop... (CustomData:%p)", data);
    data->main_loop = g_main_loop_new(data->context, FALSE);//创建Glib主循环
    check_initialization_complete(data);//检查初始化完成，通知UI
    g_main_loop_run(data->main_loop);//开始运行，直到g_main_loop_quit()
    GST_DEBUG ("Exited main loop");
    g_main_loop_unref(data->main_loop);
    data->main_loop = NULL;

    /* Free resources */
    g_main_context_pop_thread_default(data->context);
    g_main_context_unref(data->context);
    gst_element_set_state(data->pipeline, GST_STATE_NULL);
    gst_object_unref (data->video_sink);
    gst_object_unref(data->pipeline);

    return NULL;
}

/*
 * Java Bindings
 */

/* Instruct the native code to create its internal data structure, pipeline and thread */
// 初始化。创建一个专用线程、一个 GStreamer 管道、一个 GLib 主循环，
// 并且在调用 g_main_loop_run() 并进入睡眠状态之前，它警告 Java 代码本机代码已初始化并准备好接受命令。
void
gst_native_init(JNIEnv *env, jobject thiz, jstring pipeline_description) {
    CustomData *data = g_new0 (CustomData, 1);//分配内存
    SET_CUSTOM_DATA (env, thiz, custom_data_field_id, data);//传递给java层
    GST_DEBUG ("Created CustomData at %p", data);//打印日志
    data->app = (*env)->NewGlobalRef(env, thiz);//全局 Jobject 变量, 用于存储jobject thiz
    GST_DEBUG ("Created GlobalRef for app object at %p", data->app);
    //管道描述
    char *strFromJava = (char *) (*env)->GetStringUTFChars(env, pipeline_description, NULL);
    strcpy(data->pipeline_description, strFromJava);
    (*env)->ReleaseStringUTFChars(env, pipeline_description, strFromJava);

    pthread_t gst_app_thread;//线程id
    pthread_create(&gst_app_thread, NULL, &app_function, data);//开启多线程
    data->gst_app_thread = gst_app_thread;//存储线程id
}

/* Quit the main loop, remove the native thread and free resources */
//结束。退出主循环
void
gst_native_finalize(JNIEnv *env, jobject thiz) {
    CustomData *data = GET_CUSTOM_DATA (env, thiz, custom_data_field_id);
    if (!data)
        return;
    GST_DEBUG ("Quitting main loop...");
    g_main_loop_quit(data->main_loop);//退出glib主循环
    GST_DEBUG ("Waiting for thread to finish...");
    pthread_join(data->gst_app_thread, NULL);//等待线程完成 阻塞
    GST_DEBUG ("Deleting GlobalRef for app object at %p", data->app);
    (*env)->DeleteGlobalRef(env, data->app);
    GST_DEBUG ("Freeing CustomData at %p", data);
    g_free(data);
    SET_CUSTOM_DATA (env, thiz, custom_data_field_id, NULL);
    GST_DEBUG ("Done finalizing");
}

/* Set pipeline to PLAYING state */
void
gst_native_play(JNIEnv *env, jobject thiz) {
    CustomData *data = GET_CUSTOM_DATA (env, thiz, custom_data_field_id);
    if (!data)
        return;
    GST_DEBUG ("Setting state to PLAYING");
    gst_element_set_state(data->pipeline, GST_STATE_PLAYING);
}

/* Set pipeline to PAUSED state */
void
gst_native_pause(JNIEnv *env, jobject thiz) {
    CustomData *data = GET_CUSTOM_DATA (env, thiz, custom_data_field_id);
    if (!data)
        return;
    GST_DEBUG ("Setting state to PAUSED");
    gst_element_set_state(data->pipeline, GST_STATE_PAUSED);
}

void
gst_native_surface_init (JNIEnv * env, jobject thiz, jobject surface)
{
    CustomData *data = GET_CUSTOM_DATA (env, thiz, custom_data_field_id);
    if (!data)
        return;
    ANativeWindow *new_native_window = ANativeWindow_fromSurface (env, surface);
    GST_DEBUG ("Received surface %p (native window %p)", surface, new_native_window);

    //释放window
    if (data->native_window) {
        ANativeWindow_release (data->native_window);
        if (data->native_window == new_native_window) {
            GST_DEBUG ("New native window is the same as the previous one %p", data->native_window);
            if (data->video_sink) {
                gst_video_overlay_expose (GST_VIDEO_OVERLAY (data->video_sink));
            }
            return;
        } else {
            GST_DEBUG ("Released previous native window %p", data->native_window);
            data->initialized = FALSE;
        }
    }
    data->native_window = new_native_window;

    check_initialization_complete (data);
}

static void
gst_native_surface_finalize (JNIEnv * env, jobject thiz)
{
    CustomData *data = GET_CUSTOM_DATA (env, thiz, custom_data_field_id);
    if (!data)
        return;
    GST_DEBUG ("Releasing Native Window %p", data->native_window);

    if (data->video_sink) {
        //为sink清空窗口句柄
        gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (data->video_sink), (guintptr) NULL);
        //暂停播放 TODO
        //gst_element_set_state (data->pipeline, GST_STATE_READY);
        gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
    }

    ANativeWindow_release (data->native_window);
    data->native_window = NULL;
    data->initialized = FALSE;
}

/* Static class initializer: retrieve method and field IDs */
static jboolean
gst_native_class_init(JNIEnv *env, jclass klass) {
    custom_data_field_id =
            (*env)->GetFieldID(env, klass, "native_custom_data", "J");
    set_message_method_id =
            (*env)->GetMethodID(env, klass, "setMessage", "(Ljava/lang/String;)V");
    on_gstreamer_initialized_method_id =
            (*env)->GetMethodID(env, klass, "onGStreamerInitialized", "()V");

    if (!custom_data_field_id || !set_message_method_id
        || !on_gstreamer_initialized_method_id) {
        /* We emit this message through the Android log instead of the GStreamer log because the later
         * has not been initialized yet.
         */
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "The calling class does not implement all necessary interface methods");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

/* List of implemented native methods */
JNINativeMethod methods[] = {
        {"nativeInit", "(Ljava/lang/String;)V", (void *) gst_native_init},
        {"nativeFinalize",  "()V", (void *) gst_native_finalize},
        {"nativePlay",      "()V", (void *) gst_native_play},
        {"nativePause",     "()V", (void *) gst_native_pause},
        {"nativeSurfaceInit", "(Ljava/lang/Object;)V", (void *) gst_native_surface_init},
        {"nativeSurfaceFinalize", "()V", (void *) gst_native_surface_finalize},
        {"nativeClassInit", "()Z", (void *) gst_native_class_init}
};

//入口函数
jint
JNI_OnLoad(JavaVM *vm, void *reserved) {
    //全局 JavaVM 变量, 用于获取环境变量env
    java_vm = vm;

    //获取JNI env变量
    JNIEnv *env = NULL;
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    //获取native方法所在类
    jclass clazz = (*env)->FindClass(env, "org/freedesktop/gstreamer/GStreamerPlayer");
    if (clazz == NULL) {
        return -1;
    }
    // 动态注册native方法
    if ((*env)->RegisterNatives(env, clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        return -1;
    }

    //创建线程私有数据，存储环境变量env
    pthread_key_create(&current_jni_env, detach_current_thread);

    //日志相关
    GST_DEBUG_CATEGORY_INIT (debug_category, LOG_TAG, 0, LOG_TAG);
    gst_debug_set_threshold_for_name(LOG_TAG, GST_LEVEL_DEBUG);

    return JNI_VERSION_1_4;
}