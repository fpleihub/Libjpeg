//
// Created by fplei on 2018/1/25.
//

#ifndef LIBJPEG_LIBJPEG_UTILS_H
#define LIBJPEG_LIBJPEG_UTILS_H
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <android/log.h>
#define TAG "cpp_result" // 这个是自定义的LOG的标识
#define LOGI(...) __android_log_print(ANDROID_LOG_VERBOSE,TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_ERROR,TAG, __VA_ARGS__)
#ifdef __cplusplus
extern "C" {
#endif
//图片压缩
jboolean doCompress(JNIEnv *env, jclass cls, jobject bitmap,jint width, jint height,jstring fileName_, jint quality);
//压缩图片，放入out_put_stream中
void doCompress_bytes(JNIEnv *env, jclass cls,jobject bitmap,jint width, jint height,jobject out_put_stream,jint quality);
//压缩图片，以Bitmap形式返回
jobject doCompress_bitmap(JNIEnv *env, jclass cls,jobject bitmap,jint width, jint height,jint quality);
//放大缩小图片
jobject opt_big_or_small(JNIEnv *env, jclass cls,jobject bitmap,jfloat x_scale_rate,jfloat y_scale_rate);
//获取sd卡根目录
void findSDRoot(JNIEnv *env,char **root);
#ifdef __cplusplus
}
#endif
#endif //LIBJPEG_LIBJPEG_UTILS_H
