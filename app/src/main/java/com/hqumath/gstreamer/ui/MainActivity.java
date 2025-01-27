package com.hqumath.gstreamer.ui;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

import com.hqumath.gstreamer.databinding.ActivityMainBinding;

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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
    }
}
