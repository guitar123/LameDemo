package com.example.lame.lamedemo;

import android.Manifest;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

import com.example.lame.lamedemo.lame.LameUtils;

import pub.devrel.easypermissions.EasyPermissions;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private LameUtils mLameUtils;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //请求权限
        String[] perms = {Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE};
        if (!EasyPermissions.hasPermissions(this, perms)) {
            EasyPermissions.requestPermissions(this, "请求读写权限", 222, perms);
        }

        mLameUtils = new LameUtils();

        findViewById(R.id.btn_convert_wav_to_mp3).setOnClickListener(this);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        EasyPermissions.onRequestPermissionsResult(requestCode, permissions, grantResults, this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_convert_wav_to_mp3:
                convertWav2Mp3();
                break;
        }
    }

    //将wav文件转换为MP3
    private void convertWav2Mp3() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                String absolutePath = Environment.getExternalStorageDirectory().getAbsolutePath();
                mLameUtils.convertToMP3(absolutePath + "/aaa01.wav", absolutePath + "/aaa01.mp3");
                mLameUtils.getLameVersion();
            }
        }).start();
    }
}
