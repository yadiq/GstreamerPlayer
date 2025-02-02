package com.hqumath.gstreamer.ui;

import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import com.bumptech.glide.Glide;
import com.hqumath.gstreamer.R;
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
        Glide.with(this).load(R.drawable.icon_loading).into(binding.ivLoadingVideo1);

        binding.btnInit1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (videoHelper1 == null) {
                    videoHelper1 = new GStreamerPlayer();
                    videoHelper1.init(defaultMediaUri, binding.surface1.getHolder(), new GStreamerPlayer.VideoListener() {
                        @Override
                        public void showLoading(boolean isShow) {
                            if (isShow) {
                                binding.getRoot().post(() -> binding.ivLoadingVideo1.setVisibility(View.VISIBLE));
                            } else {
                                binding.getRoot().postDelayed(() -> binding.ivLoadingVideo1.setVisibility(View.GONE), 500);//低性能手机，loading隐藏失败
                            }
                        }

                        @Override
                        public void showNoSignal(boolean isShow) {

                        }
                    });
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
