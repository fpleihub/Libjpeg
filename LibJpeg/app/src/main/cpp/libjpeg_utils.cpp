//
// Created by fplei on 2018/1/17.
//
#include <android/bitmap.h>
#include <string.h>
#include <stdio.h>
extern "C"{
#include "include/libjpeg_utils.h"
#include "include/jpeg/cdjpeg.h"
#include "include/jpeg/jpeglib.h"
#include <setjmp.h>
#include "include/jpeg/jinclude.h"
#include "include/jpeg/jerror.h"
#include "include/jpeg/cderror.h"
#include "include/jpeg/android/jconfig.h"
}
extern "C"
typedef uint8_t BYTE;
struct my_error_mgr {
    struct jpeg_error_mgr pub;	/* "public" fields */
    jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct my_error_mgr * my_error_ptr;
METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message) (cinfo);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}

struct JPEGINFO {
    unsigned int width;
    unsigned int height;
    unsigned int colortype;
    unsigned char* dstImg;
};
int readjpeg(const  char *file_name,JPEGINFO &jpeginfo)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    FILE * infile;
    JSAMPARRAY buffer;
    int row_stride;
    if ((infile = fopen(file_name, "rb")) == NULL) {
        LOGI("can't open %s\n", file_name);
        return 0;
    }
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return 0;
    }
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    (void)jpeg_read_header(&cinfo, TRUE);

    (void)jpeg_start_decompress(&cinfo);
    row_stride = cinfo.output_width * cinfo.output_components;

    int cols = cinfo.output_width;
    int rows = cinfo.output_height;

    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
    jpeginfo.width = cinfo.output_width;
    jpeginfo.height = cinfo.output_height;
    if (cinfo.output_components == 3)
    {
        jpeginfo.colortype=3;
        jpeginfo.dstImg = new unsigned char[jpeginfo.width*jpeginfo.height * 3+1];

    }else{
        jpeginfo.colortype=1;
        jpeginfo.dstImg = new unsigned char[jpeginfo.width * jpeginfo.height+1];
    }
    while (cinfo.output_scanline < cinfo.output_height) {
        (void)jpeg_read_scanlines(&cinfo, buffer, 1);
        for (size_t i = 0; i < cinfo.output_width; i++)
        {
            //rgb
            if (jpeginfo.colortype == 3)
            {
                jpeginfo.dstImg[(cinfo.output_scanline - 1)*jpeginfo.width*3 + i*3] = buffer[0][i * 3];
                jpeginfo.dstImg[(cinfo.output_scanline - 1)*jpeginfo.width*3 + i*3 + 1] = buffer[0][i * 3 + 1];
                jpeginfo.dstImg[(cinfo.output_scanline - 1)*jpeginfo.width *3+ i*3 + 2] = buffer[0][i * 3 + 2];
            }
            else if (jpeginfo.colortype == 1)
            {
                jpeginfo.dstImg[(cinfo.output_scanline - 1)*jpeginfo.width + i] = buffer[0][i];
            }
        }
    }
    (void)jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 1;
}
//jpg文件->jpg内存数据
BYTE *jpgfile_to_jpgmem(const char *jpg_file)
{
    FILE *fp = fopen(jpg_file,"rb");
    if(fp == NULL) return NULL;
    fseek(fp,0,SEEK_END);
    int length = ftell(fp);
    fseek(fp,0,SEEK_SET);
    BYTE *jpg=new BYTE[length];
    fread(&jpg,length,1,fp);
    fclose(fp);
    LOGI("jpg.len=%d",SIZEOF(jpg));
    return (BYTE *) jpg;
}
//jpg内存数据->jpg文件
void saveJpegFile(char *jpg_file,BYTE *jpg,int size)
{
    FILE *fp = fopen(jpg_file,"wb+");
    if(fp == NULL) return;
    fwrite(jpg,size,1,fp);
    fclose(fp);
}
int codeJPEG(BYTE *data, int w, int h, int quality, const char *outfilename, jboolean optimize) {
    LOGI("generateJPEG");
    //3
    int nComponent = 3;
    struct jpeg_compress_struct jcs;
    struct jpeg_error_mgr jem;
    jcs.err = jpeg_std_error(&jem);
    //为JPEG对象分配空间并初始化
    jpeg_create_compress(&jcs);
    //获取文件信息
    FILE *f = fopen(outfilename, "wb+");
    if (f == NULL) {
        return 0;
    }
    //指定压缩数据源
    jpeg_stdio_dest(&jcs, f);
    jcs.image_width = w;//image_width->JDIMENSION->typedef unsigned int
    jcs.image_height = h;
    jcs.arith_code = true;
    //input_components为1代表灰度图，在等于3时代表彩色位图图像
    jcs.input_components = nComponent;
    if (nComponent == 1)
        //in_color_space为JCS_GRAYSCALE表示灰度图，在等于JCS_RGB时代表彩色位图图像
        jcs.in_color_space = JCS_GRAYSCALE;
    else
        jcs.in_color_space = JCS_RGB;

    jpeg_set_defaults(&jcs);
    //optimize_coding为TRUE，将会使得压缩图像过程中基于图像数据计算哈弗曼表，由于这个计算会显著消耗空间和时间，默认值被设置为FALSE。
    jcs.optimize_coding = optimize;
    //为压缩设定参数，包括图像大小，颜色空间
    jpeg_set_quality(&jcs, quality, true);
    //开始压缩
    jpeg_start_compress(&jcs, TRUE);
    JSAMPROW row_pointer[1];//JSAMPROW就是一个字符型指针 定义一个变量就等价于=========unsigned char *temp
    int row_stride;
    //计算每行需要的空间大小，比如RGB图像就是宽度×3，灰度图就是宽度×1
    row_stride = jcs.image_width * nComponent;
    while (jcs.next_scanline < jcs.image_height) {
        row_pointer[0] = &data[jcs.next_scanline * row_stride];
        jpeg_write_scanlines(&jcs, row_pointer, 1);
    }
    //压缩完毕
    jpeg_finish_compress(&jcs);
    //释放资源
    jpeg_destroy_compress(&jcs);
    fclose(f);
    return 1;
}

void doCompress_bytes(JNIEnv *env, jclass cls,jobject bitmap,jint width, jint height,jobject out_put_stream,jint quality){
    LOGI("doCompress_bytes");
    char *filename=NULL;
    findSDRoot(env,&filename);
    long seed=random();
    strcat(filename,"/temp.jpg");
    LOGI("filename=%s",filename);
    LOGI("quality=%d",quality);
    AndroidBitmapInfo infoColor;
    BYTE *pixelColor;
    BYTE *data;
    BYTE *tempData;
    if ((AndroidBitmap_getInfo(env, bitmap, &infoColor)) < 0) {
        LOGI("解析错误");
    }
    if ((AndroidBitmap_lockPixels(env, bitmap, (void **) &pixelColor)) < 0) {
        LOGI("加载失败");
    }
    BYTE r, g, b;
    int color,format,w,h;
    w=width;
    h=height;
    //如果传入的宽高小于0
    if (w<=0){
        w=static_cast<int>(infoColor.width);
    }
    if (h<=0){
        h=static_cast<int>(infoColor.height);
    }
    LOGI("width=%d",w);
    LOGI("height=%d",h);
    format = infoColor.format;
    data = (BYTE *) malloc(w * h * 3);
    tempData = data;
    //解析每一个像素点里面的rgb值(去掉alpha值)，保存到一维数组data里面
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (format == ANDROID_BITMAP_FORMAT_RGBA_8888){
                ////获取二维数组的每一个像素信息首地址
                color = *((int *) pixelColor);
                r = ((color & 0x00FF0000)>> 16);
                g = ((color & 0x0000FF00) >> 8);
                b = color & 0X000000FF;

                *data = b;
                *(data + 1) = g;
                *(data + 2) = r;
                data += 3;
                //一个像素包括argb四个值，每+4就是取下一个像素点
                pixelColor += 4;
            } else{
                break;
            }
        }
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    int resultCode = codeJPEG(tempData, w, h, quality, filename, true);
    LOGI("resultCode=%d",resultCode);
    if(resultCode==1){
        JPEGINFO jpeginfo;
        int flag=readjpeg(filename,jpeginfo);
        LOGI("readjpeg->flag=%d",flag);
        if (&jpeginfo!=NULL){
            const jbyte *by = (jbyte*)jpeginfo.dstImg;
            jsize size=jpeginfo.width*jpeginfo.height;
            jbyteArray jarray = env->NewByteArray(size);
            env->SetByteArrayRegion(jarray, 0, size, by);
            jclass  byteArrayOutputStream=env->GetObjectClass(out_put_stream);
            jmethodID writeId=env->GetMethodID(byteArrayOutputStream,"write","([B)V");
            env->CallVoidMethod(out_put_stream,writeId,jarray);
        }
    }
    free(tempData);
}
jobject doCompress_bitmap(JNIEnv *env, jclass cls,jobject bitmap,jint width, jint height,jint quality){
    LOGI("doCompress_bitmap");
    return NULL;
}
//缩放图片
jobject opt_image(JNIEnv *env,jobject bitmap,jint x,jint y,jint width,jint height,jfloat x_scale_rate,jfloat y_scale_rate){
    if (width<=0 || height<=0 || x_scale_rate<0 || y_scale_rate<0 || x_scale_rate>2 ||y_scale_rate>2){
        LOGI("缩放参数不正确！");
        return NULL;
    }
    jclass  matrix_class=env->FindClass("android/graphics/Matrix");
    //查找该类的构造方法
    jmethodID mid = env->GetMethodID(matrix_class, "<init>", "()V");
    jobject  matrix=env->NewObject(matrix_class,mid);
    jmethodID method_scale=env->GetMethodID(env->GetObjectClass(matrix),"postScale","(FF)Z");
    LOGI("x_scale_rate=%f,y_scale_rate=%f",x_scale_rate,y_scale_rate);
    env->CallBooleanMethod(matrix,method_scale,x_scale_rate,y_scale_rate);
    jclass  bitmap_class=env->GetObjectClass(bitmap);
    jmethodID createBitmapID=env->GetStaticMethodID(bitmap_class,"createBitmap","(Landroid/graphics/Bitmap;IIIILandroid/graphics/Matrix;Z)Landroid/graphics/Bitmap;");
    LOGI("GetMethodID::createBitmapID,x=%d,y=%d,width=%d,height=%d",x,y,width,height);
    jobject newBitmap=env->CallStaticObjectMethod(bitmap_class,createBitmapID,bitmap,x,y,width,height,matrix,JNI_TRUE);
    jthrowable err = env->ExceptionOccurred();
    if (err != NULL){
        //手动清空异常信息，保证Java代码能够继续执行
        LOGI("opt_image->error:%o",err);
        env->Throw(err);
        env->ExceptionClear();
        //提供补救措施，例如获取另外一个属性
    }
    LOGI("CallStaticObjectMethod::createBitmap");
    env->DeleteLocalRef(matrix);
    return newBitmap;
}

jboolean doCompress(JNIEnv *env, jclass cls, jobject bitmap,
                    jint width, jint height,
                    jstring fileName_, jint quality){
    AndroidBitmapInfo infoColor;
    BYTE *pixelColor;
    BYTE *data;
    BYTE *tempData;
    const char *filename = env->GetStringUTFChars(fileName_, 0);
    LOGI("filename=%s",filename);
    LOGI("quality=%d",quality);
    if ((AndroidBitmap_getInfo(env, bitmap, &infoColor)) < 0) {
        LOGI("解析错误");
        return false;
    }
    if ((AndroidBitmap_lockPixels(env, bitmap, (void **) &pixelColor)) < 0) {
        LOGI("加载失败");
        return false;
    }
    BYTE r, g, b;
    int color,format,w,h;
    w=width;
    h=height;
    //如果传入的宽高小于0
    if (w<=0){
        w=static_cast<int>(infoColor.width);
    }
    if (h<=0){
        h=static_cast<int>(infoColor.height);
    }
    LOGI("width=%d",w);
    LOGI("height=%d",h);
    format = infoColor.format;
    data = (BYTE *) malloc(w * h * 3);
    tempData = data;
    //解析每一个像素点里面的rgb值(去掉alpha值)，保存到一维数组data里面
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (format == ANDROID_BITMAP_FORMAT_RGBA_8888){
                ////获取二维数组的每一个像素信息首地址
                color = *((int *) pixelColor);
                r = ((color & 0x00FF0000)>> 16);
                g = ((color & 0x0000FF00) >> 8);
                b = color & 0X000000FF;

                *data = b;
                *(data + 1) = g;
                *(data + 2) = r;
                data += 3;
                //一个像素包括argb四个值，每+4就是取下一个像素点
                pixelColor += 4;
            } else{
                break;
                return JNI_FALSE;
            }
        }
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    int resultCode = codeJPEG(tempData, w, h, quality, filename, true);
    LOGI("resultCode=%d",resultCode);
    free(tempData);
    if (resultCode == 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}
jobject opt_big_or_small(JNIEnv *env, jclass cls,jobject bitmap,jfloat x_scale_rate,jfloat y_scale_rate){
    jobject obj;
    //big image
    AndroidBitmapInfo bitmapInfo;
    if(AndroidBitmap_getInfo(env,bitmap,&bitmapInfo)<0){
        return NULL;
    }
    obj=opt_image(env,bitmap,0,0,bitmapInfo.width,bitmapInfo.height,x_scale_rate,y_scale_rate);
    LOGI("recycling bitmap...");
    jclass bitmapCls = env->GetObjectClass(bitmap);
    jmethodID recycleFunction = env->GetMethodID(bitmapCls, "recycle", "()V");
    if (recycleFunction == 0)
    {
        LOGI("error recycling!");
        return NULL;
    }
    env->CallVoidMethod(bitmap, recycleFunction);
    if(AndroidBitmap_getInfo(env,obj,&bitmapInfo)<0){
        return NULL;
    }
    LOGI("bitmapInfo.width=%d,bitmapInfo.height=%d",bitmapInfo.width,bitmapInfo.height);
    return obj;
}
void findSDRoot(JNIEnv *env,char **root){
    jclass envcls = env->FindClass("android/os/Environment"); //获得类引用
    if (envcls != nullptr){
        //找到对应的类，该类是静态的返回值是File
        jmethodID id = env->GetStaticMethodID(envcls, "getExternalStorageDirectory", "()Ljava/io/File;");
        jobject fileObj = env->CallStaticObjectMethod(envcls,id,"");
        //通过上述方法返回的对象创建一个引用即File对象
        jclass flieClass = env->GetObjectClass(fileObj); //或得类引用
        //在调用File对象的getPath()方法获取该方法的ID，返回值为String 参数为空
        jmethodID getpathId = env->GetMethodID(flieClass, "getPath", "()Ljava/lang/String;");
        //调用该方法及最终获得存储卡的根目录
        jstring pathStr = (jstring)env->CallObjectMethod(fileObj,getpathId,"");
        const  char *path = env->GetStringUTFChars(pathStr,NULL);
        *root= (char *) path;
    }
}
static const JNINativeMethod gMethods[]={
        {"doCompress4File","(Landroid/graphics/Bitmap;IILjava/lang/String;I)Z",(jboolean*)doCompress},
        {"doCompress4Bitmap","(Landroid/graphics/Bitmap;III)Landroid/graphics/Bitmap;",(jobject *)doCompress_bitmap},
        {"doCompress4Byte","(Landroid/graphics/Bitmap;IILjava/io/ByteArrayOutputStream;I)V",(void*)doCompress_bytes},
        {"opt_big_or_small","(Landroid/graphics/Bitmap;FF)Landroid/graphics/Bitmap;",(jobject *)opt_big_or_small}
};
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved){
    JNIEnv* env = NULL; //注册时在JNIEnv中实现的，所以必须首先获取它
    jint result = -1;
    const char* const kClassName="libjpeg/handpay/com/libjpeg/ImageCompressHelper";
    if(vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) //从JavaVM获取JNIEnv，一般使用1.4的版本
        return -1;
    jclass myClass = env->FindClass(kClassName);
    if(myClass == NULL)
    {
        LOGI("cannot get class:%s\n", kClassName);
        return -1;
    }
    if(env->RegisterNatives(myClass,gMethods,sizeof(gMethods)/sizeof(gMethods[0]))<0)
    {
        LOGI("register native method failed!\n");
        return -1;
    }
    LOGI("--------JNI_OnLoad-----");
    return JNI_VERSION_1_4; //这里很重要，必须返回版本，否则加载会失败。
}
JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved){
    LOGI("--------JNI_OnUnload-----");
}