package com.hqumath.gstreamer.app;

/**
 * ****************************************************************
 * 作    者: Created by gyd
 * 创建时间: 2019/1/22 14:30
 * 注意事项:
 * ****************************************************************
 */
public class Constants {

    //音视频
    public static boolean gstIsInitialized;//GStreamer是否初始化
    public static final String localhost = "127.0.0.1";//本机IP
    public static String videoCache = "1000"; //默认1000  最小200
    public static final int videoPort = 10000;//车视频端口
    public static final int audioPort = 20000;//车拾音端口
}
