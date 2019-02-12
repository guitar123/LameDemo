#include <jni.h>
#include <string>
#include "android/log.h"
#include <stdio.h>
#include "lame/lame.h"


#ifndef LOGTAG
#define LOGTAG "jni_log_tag"
#define null NULL

//定义可变参数的宏
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOGTAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOGTAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOGTAG,__VA_ARGS__)

#endif


#ifdef __cplusplus
extern "C" {
#endif

//定义类路径
static const char *class_name = "com/example/lame/lamedemo/lame/LameUtils";

void notifyProgressChanged(JNIEnv *env, jobject instance, int read, int total) {
    LOGD("notifyProgressChanged(read = %d, total = %d)", read, total);
    jclass aClass = env->FindClass(class_name);
    jmethodID pJmethodID = env->GetMethodID(aClass, "onConvertProgressChangedWithLame", "(I)V");
    env->CallVoidMethod(instance, pJmethodID, read);
    env->DeleteLocalRef(aClass);
}

/**
 * 将jstring 转换为char* 编码格式是 gb2312
 */
char *jString2CString(JNIEnv *env, jstring str) {
    char *ch = null;

    jclass pJStringClass = env->FindClass("java/lang/String");//获取java String class
    jstring pJStringCode = env->NewStringUTF("gb2312");//创建gb2312的java string

    // byte[] = getBytes(charsetName)
    jmethodID pJmethodID = env->GetMethodID(pJStringClass, "getBytes", "(Ljava/lang/String;)[B");

    jbyteArray pJByteArray = static_cast<jbyteArray>(env->CallObjectMethod(str, pJmethodID,
                                                                           pJStringCode));

    //获取 jbyteArray 的长度
    jsize length = env->GetArrayLength(pJByteArray);

    jbyte *jByte = env->GetByteArrayElements(pJByteArray, JNI_FALSE);
    if (length > 0) {
        ch = static_cast<char *>(malloc(length + 1));
        memcpy(ch, jByte, length);
        ch[length] = 0;
    }
    env->ReleaseByteArrayElements(pJByteArray, jByte, 0);
    return ch;
}

/**
 * 转换为mp3
 */
void convertToMP3(JNIEnv *env, jobject instance,
                  jstring wav_path_, jstring mp3_path_) {
    //将JString转换为C char*
    const char *wav_path_1 = env->GetStringUTFChars(wav_path_, 0);
    const char *mp3_path_1 = env->GetStringUTFChars(mp3_path_, 0);


//    const char *wav_path_2 = jString2CString(env, wav_path_);
//    const char *mp3_path_2 = jString2CString(env, mp3_path_);
    LOGD("convertToMP3(%s,%s)", wav_path_1, mp3_path_1);

    FILE *wav_file = fopen(wav_path_1, "rb");
    FILE *mp3_file = fopen(mp3_path_1, "wb");
    if (wav_file == null || mp3_file == null) {
        if (wav_file == null) LOGD("wav_file == null");
        if (mp3_file == null) LOGD("mp3_file == null");
        return;
    }

    lame_t lame = lame_init();//初始化lame
    lame_set_in_samplerate(lame, 44100);//配置采样率
    lame_set_num_channels(lame, 2);//配置声道数目
    lame_set_VBR(lame, vbr_default);
    lame_init_params(lame);

    LOGD("lame init success ...");

    const int BUF = 8192;
    short int wav_buffer[BUF * 2];
    unsigned char mp3_buffer[BUF];

    //读写操作
    int read = 0, write = 0;
    int total = 0;
    do {
        read = fread(wav_buffer, sizeof(short int) * 2, BUF, wav_file);
        total += read * sizeof(short int) * 2;
        notifyProgressChanged(env, instance, read, total);

        if (read != 0) {
            //编码
            write = lame_encode_buffer_interleaved(lame, wav_buffer, read, mp3_buffer,
                                                   sizeof(mp3_buffer));
            //写文件
            fwrite(mp3_buffer, sizeof(unsigned char), write, mp3_file);
        }

        if (read == 0) {
            lame_encode_flush(lame, mp3_buffer, sizeof(mp3_buffer));
        }
    } while (read != 0);

    //关闭资源
    lame_close(lame);
    fclose(wav_file);
    fclose(mp3_file);

    env->ReleaseStringUTFChars(wav_path_, wav_path_1);
    env->ReleaseStringUTFChars(mp3_path_, mp3_path_1);
}

void convertToMp3WithLame(JNIEnv *env, jobject instance, jstring wav_path_, jstring mp3_path_) {
    const char *wav_path = env->GetStringUTFChars(wav_path_, false);
    const char *mp3_path = env->GetStringUTFChars(mp3_path_, false);

    FILE *wav_file = fopen(wav_path, "rb");
    if (wav_file) {
        LOGD("wavfile == null");
        return;
    }
    FILE *mp3_file = fopen(mp3_path, "wb");
    if (mp3_file) {
        LOGD("mp3file == null");
        return;
    }

    LOGD("Lame init ...");
    //初始化lame
    lame_t lame = lame_init();
    lame_set_in_samplerate(lame, 44100);
    lame_set_out_samplerate(lame, 44100);
    lame_set_num_channels(lame, 2);
    lame_set_VBR(lame, vbr_default);
    lame_init_params(lame);

    //创建缓冲区进行读写操作
    const int BUFFER_SIZE = 1024 * 256;
    short *buffer = new short[BUFFER_SIZE / 2];
    short *left_buffer = new short[BUFFER_SIZE / 4];
    short *right_buffer = new short[BUFFER_SIZE / 4];
    unsigned char *mp3_buffer = new unsigned char[BUFFER_SIZE];
    size_t readBufferSize = 0;
    //buffer:读取出来的数据存放的缓冲区 第二个参数:块儿数据的大小 第三个参数:块儿的数目
    while ((readBufferSize = fread(buffer, 2, BUFFER_SIZE / 2, wav_file)) > 0) {
        for (int i = 0; i < readBufferSize; ++i) {
            if (i % 2 == 0) {
                left_buffer[i / 2] = buffer[i];
            } else {
                right_buffer[i / 2] = buffer[i];
            }
            //编码
            size_t write_size = static_cast<size_t>(lame_encode_buffer(lame, left_buffer,
                                                                       right_buffer,
                                                                       (int) (readBufferSize / 2),
                                                                       mp3_buffer,
                                                                       BUFFER_SIZE));
            //写文件
            fwrite(mp3_buffer, 1, write_size, mp3_file);
        }
        delete[] buffer;
        delete[] left_buffer;
        delete[] right_buffer;
        delete[] mp3_buffer;

        fclose(wav_file);
        fclose(mp3_file);
        lame_close(lame);
    }
}

/**
 * 获取version
 */
jstring getLameVersion(JNIEnv *env, jobject instance) {
    LOGI("getLameVersion %s", get_lame_version());
    return env->NewStringUTF(get_lame_version());
}


//动态注册函数
static JNINativeMethod jni_Methods_table[] = {
        {"convertToMP3",   "(Ljava/lang/String;Ljava/lang/String;)V", (void *) convertToMP3},
        {"getLameVersion", "()Ljava/lang/String;",                    (void *) getLameVersion}
};


//动态注册
int registerNativeMethods(JNIEnv *env, const char *class_name, const JNINativeMethod *gMethods,
                          int numMethods) {
    jclass clazz = env->FindClass(class_name);
    LOGD("registerNativeMethods %s", class_name);
    if (clazz == null) {
        LOGE("registerNativeMethods Error : clazz = null classname = %s", class_name);
        return JNI_ERR;
    }

    if ((*env).RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("(*env).RegisterNatives Error : classname = %s", class_name);
        return JNI_ERR;
    }

    (env)->DeleteLocalRef(clazz);
    return JNI_OK;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGD("JNI_OnLoad");
    JNIEnv *env = null;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("vm->GetEnv : JNI_ERROR");
        return JNI_EVERSION;
    }
    registerNativeMethods(env, class_name, jni_Methods_table,
                          sizeof(jni_Methods_table) / sizeof(JNINativeMethod));
    return JNI_VERSION_1_4;
}

#ifdef __cplusplus
}
#endif