package com.hqumath.gstreamer.ui;

import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import com.hqumath.gstreamer.databinding.ActivityMainBinding;
import com.hqumath.gstreamer.utils.CommonUtil;

import org.freedesktop.gstreamer.GStreamerPlayer;

/**
 * ****************************************************************
 * 作    者: Created by gyd
 * 创建时间: 2025/1/27 20:51
 * 文件描述:
 * 注意事项:
 * ****************************************************************
 */
public class MainActivity extends AppCompatActivity {
    private ActivityMainBinding binding;
    private GStreamerPlayer videoHelper1;

    private String defaultMediaUri = "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.ogv";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        CommonUtil.init(this);
//        binding.tvValue.setText(GStreamerPlayer.getVersion());

        binding.btnInit1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (videoHelper1 == null) {
                    videoHelper1 = new GStreamerPlayer();
                    videoHelper1.init(defaultMediaUri, binding.surface1.getHolder(), null);
                    videoHelper1.surfaceInit();
                    CommonUtil.toast("初始化视频1");
                }
            }
        });
        binding.btnClose1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (videoHelper1 != null) {
                    videoHelper1.stop();
                    videoHelper1.surfaceDestroyed();
                    videoHelper1.close();
                    videoHelper1 = null;
                    CommonUtil.toast("释放视频1");
                }
            }
        });
        binding.btnInit1.performClick();
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (videoHelper1 != null) {
            videoHelper1.start();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (videoHelper1 != null) {
            videoHelper1.stop();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (videoHelper1 != null) {
            videoHelper1.close();
            videoHelper1 = null;
        }
    }
}
