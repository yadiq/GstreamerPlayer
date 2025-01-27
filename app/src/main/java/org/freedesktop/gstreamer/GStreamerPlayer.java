package org.freedesktop.gstreamer;

/**
 * ****************************************************************
 * 作    者: Created by gyd
 * 创建时间: 2025/1/27 22:29
 * 文件描述:
 * 注意事项:
 * ****************************************************************
 */
public class GStreamerPlayer {
    static {
        System.loadLibrary("gstreamer_android");
        System.loadLibrary("gstreamer_player");
    }

    public static native String getVersion();

}
