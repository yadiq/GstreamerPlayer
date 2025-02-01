package org.freedesktop.gstreamer;

import android.view.SurfaceHolder;

import com.hqumath.gstreamer.app.Constants;
import com.hqumath.gstreamer.utils.CommonUtil;
import com.hqumath.gstreamer.utils.LogUtil;

/**
 * ****************************************************************
 * 作    者: Created by gyd
 * 创建时间: 2025/1/27 22:29
 * 文件描述:
 * 注意事项:
 * ****************************************************************
 */
public class GStreamerPlayer {
    ////////////////////////////////////native///////////////////////////////////
    static {
        System.loadLibrary("gstreamer_android");
        System.loadLibrary("gstreamer_player");
        nativeClassInit();//初始化java层变量和方法
    }

    private native void nativeInit(String pipeline_description);     // Initialize native code, build pipeline, etc

    private native void nativeFinalize(); // Destroy pipeline and shutdown native code

    private native void nativePlay();     // Set pipeline to PLAYING

    private native void nativePause();    // Set pipeline to PAUSED

    private static native boolean nativeClassInit();//初始化java层变量和方法

    private native void nativeSurfaceInit(Object surface);

    private native void nativeSurfaceFinalize();

    private long native_custom_data;      // Native code will use this to keep private data

    // Called from native code. This sets the content of the TextView from the UI thread.
    private void setMessage(final String message) {
        LogUtil.d(TAG, "setMessage: " + message);
        if (message.contains("PLAYING")) {//隐藏loading
            if (listener != null) listener.showLoading(false);
        }
    }

    // Called from native code. Native code calls this once it has created its pipeline and
    // the main loop is running, so it is ready to accept commands.
    private void onGStreamerInitialized() {
        LogUtil.d(TAG, "onGStreamerInitialized");
        if (listener != null) listener.showLoading(true);//显示loading
    }

    ////////////////////////////////////java///////////////////////////////////
    private final String TAG = "GStreamer";
    private String url;
    private SurfaceHolder surfaceHolder;
    private VideoListener listener;

    public GStreamerPlayer() {
        if (!Constants.gstIsInitialized) {
            Constants.gstIsInitialized = true;
            try {
                GStreamer.init(CommonUtil.getContext());
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 初始化视频
     */
    public void init(String url, SurfaceHolder surfaceHolder, VideoListener listener) {
        this.url = url;
        this.surfaceHolder = surfaceHolder;
        this.listener = listener;
        surfaceHolder.addCallback(surfaceHolderCallback);

        nativeInit(getConfig());
    }

    //管道命令
    private String getConfig() {
        String config = "playbin uri=" + url;
        return config;
    }

    public void start() {
        nativePlay();
    }

    public void stop() {
        nativePause();
    }

    public void close() {
        nativeFinalize();
        if (surfaceHolder != null) {
            surfaceHolder.removeCallback(surfaceHolderCallback);
            surfaceHolder = null;
        }
        listener = null;

    }

    private SurfaceHolder.Callback surfaceHolderCallback = new SurfaceHolder.Callback() {
        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            LogUtil.d(TAG, "Surface created");
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            LogUtil.d(TAG, "Surface changed to format " + format + " width " + width + " height " + height);
            nativeSurfaceInit(holder.getSurface());//自动执行 nativePlay()
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            LogUtil.d(TAG, "Surface destroyed");
            nativeSurfaceFinalize();//自动执行 nativePause()
        }
    };

    /**
     * surface初始化
     * surfaceChanged未执行时，需手动执行
     */
    public void surfaceInit() {
        if (surfaceHolder != null) {
            nativeSurfaceInit(surfaceHolder.getSurface());
        }
    }

    /**
     * surface释放
     * surfaceDestroyed未执行时，需手动执行
     */
    public void surfaceDestroyed() {
        nativeSurfaceFinalize();
    }

    public interface VideoListener {
        void showLoading(boolean isShow);//加载中

        void showNoSignal(boolean isShow);//无视频信号
    }
}
