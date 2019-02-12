package com.example.lame.lamedemo.lame;

import android.util.Log;

public class LameUtils {

    static {
        System.loadLibrary("native-lib");
    }

    /**
     * 将指定的路径的文件转换为mp3文件
     */
    public native void convertToMP3(String wav_path, String mp3_path);

    /**
     * 获取lame的版本号
     */
    public native String getLameVersion();


    /**
     * 转换进度的回掉函数
     */
    public void onConvertProgressChangedWithLame(int value) {
        Log.i("fhp", "onConvertProgressChangedWithLame() -> value = " + value + " thread-name = " + Thread.currentThread().getName());
    }
}
