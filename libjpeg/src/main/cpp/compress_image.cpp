//
// Created by chenj on 2017/8/21.
//


#include <string.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <stdio.h>
#include <setjmp.h>
#include <math.h>
#include <stdint.h>
#include <time.h>


//统一编译方式
extern "C"
{
#include "jpeg/jpeglib.h"
#include "jpeg/cdjpeg.h"  /* Common decls for cjpeg/djpeg applications */
#include "jpeg/jversion.h" /* for version message */
#include "jpeg/jconfig.h"
}

// log打印
#define LOG_TAG "jni"
#define LOGW(...) __android_log_write(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define true 1
#define false 0
typedef uint8_t BYTE;

// error 结构体
char *error;
struct my_error_mgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};
typedef struct my_error_mgr *my_error_ptr;

METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr) cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    error = (char *) myerr->pub.jpeg_message_table[myerr->pub.msg_code];
    LOGE("jpeg_message_table[%d]:%s", myerr->pub.msg_code,
         myerr->pub.jpeg_message_table[myerr->pub.msg_code]);
    longjmp(myerr->setjmp_buffer, 1);
}



extern "C"
int generateJPEG(BYTE *data, int w, int h, int quality, const char *outfilename, jboolean optimize)
{
    // 结构体相当于Java类
    struct jpeg_compress_struct jcs;
    //当读完整个文件的时候就会回调my_error_exit这个退出方法。
    struct my_error_mgr jem;
    jcs.err = jpeg_std_error(&jem.pub);
    jem.pub.error_exit = my_error_exit;
    // setjmp是一个系统级函数，是一个回调。
    if (setjmp(jem.setjmp_buffer))
    {
        return 0;
    }
    //初始化jsc结构体
    jpeg_create_compress(&jcs);
    //打开输出文件 wb 可写 rb 可读
    FILE *f = fopen(outfilename, "wb");
    if (f == NULL)
    {
        return 0;
    }
    //设置结构体的文件路径，以及宽高
    jpeg_stdio_dest(&jcs, f);
    jcs.image_width = w;
    jcs.image_height = h;
    // /* TRUE=arithmetic coding, FALSE=Huffman */
    jcs.arith_code = false;
    int nComponent = 3;
    /* 颜色的组成 rgb，三个 # of color components in input image */
    jcs.input_components = nComponent;
    //设置颜色空间为rgb
    jcs.in_color_space = JCS_RGB;
    ///* Default parameter setup for compression */
    jpeg_set_defaults(&jcs);
    //是否采用哈弗曼
    jcs.optimize_coding = optimize;
    //设置质量
    jpeg_set_quality(&jcs, quality, true);
    //开始压缩
    jpeg_start_compress(&jcs, TRUE);
    JSAMPROW row_pointer[1];
    int row_stride;
    row_stride = jcs.image_width * nComponent;
    while (jcs.next_scanline < jcs.image_height)
    {
        //得到一行的首地址
        row_pointer[0] = &data[jcs.next_scanline * row_stride];
        jpeg_write_scanlines(&jcs, row_pointer, 1);
    }
    /// / 压缩结束
    jpeg_finish_compress(&jcs);
    // 销毁回收内存
    jpeg_destroy_compress(&jcs);
    //关闭文件
    fclose(f);
    return 1;
}
/**
 * 通过获取Bitmap的字节数组，去除安卓中带有的alpha值（libjpeg压缩方法中传递的字节数组中只是图片的rgb没有a），然后通过调用libjpeg库中的压缩方法
 * 压缩图片到指定的文件路径
 */
extern "C"
int compress_image(JNIEnv *env, jclass type, jobject bitmap,
                   jint quality, jstring fileName_,
                   jboolean optimize,
                   jboolean recycle)
{
    //1.获取Bitmap信息
    AndroidBitmapInfo android_bitmap_info;
    AndroidBitmap_getInfo(env,bitmap,&android_bitmap_info);
    //获取bitmap的款，高，format
    int bitmap_width = android_bitmap_info.width;
    int bitmap_height = android_bitmap_info.height;
    int bitmap_format = android_bitmap_info.format;

    if(bitmap_format != ANDROID_BITMAP_FORMAT_RGBA_8888)
    {
        return -2;
    }

    //2.解析Bitmap的像素信息，并转换成RGB数据，保存到二维byte数组中
    BYTE *pixels_color;
    //2.1锁定画布
    AndroidBitmap_lockPixels(env, bitmap, (void **) &pixels_color);
    //2.2解析初始化参数值
    BYTE *data;
    BYTE r,g,b;
    //每一个像素都有三个信息RGB
    data = (BYTE *) malloc(bitmap_width * bitmap_height * 3);
    //临时保存data的首地址
    BYTE *tempData = data;
    int i = 0, j = 0;
    int color;
    //2.3解析每一个像素里面的rgb值（去掉alpha值），保存到一维数组data里面
    for(i = 0;i<bitmap_height;i++)
    {
        for(j=0;j<bitmap_width;j++) {
            color = *(int *) pixels_color;
            r = (color & 0x00FF0000) >> 16;
            g = (color & 0x0000FF00) >> 8;
            b = (color & 0x000000FF);

            *data = b;
            *(data + 1) = g;
            *(data + 2) = r;
            data = data + 3;
            //一个像素包括argb四个值，每+4就是取下一个像素点
            pixels_color += 4;
        }
    }
    //2.4解锁Bitmap
    AndroidBitmap_unlockPixels(env, bitmap);
    const char *fileName = env->GetStringUTFChars(fileName_, 0);
    //3.调用libjpeg核心方法实现压缩
    int result_code = generateJPEG(tempData, bitmap_width, bitmap_height, quality, fileName,
                                   optimize);

    if(recycle){
        //4.释放资源
        env->ReleaseStringUTFChars(fileName_, fileName);
        free((void *) tempData);
        //4.2 释放Bitmap
        //4.2.1通过对象获取类
        jclass bitmap_class = env->GetObjectClass(bitmap);
        //4.2.2通过类和方法签名获取方法id
        jmethodID recycler_mid = env->GetMethodID(bitmap_class,"recycle","()V");
        //4.2.3执行回收释放方法
        env->CallVoidMethod(bitmap,recycler_mid);
    }

    //5.返回结果
    if(result_code == 0)
    {
        return -1;
    }

    return 1;
}

/**
 * 压缩图片，压缩后会释放Bitmap对象
 * quality: 0 > quality < 100
 * optimize: true->使用哈夫曼编码，false->不适用哈夫曼编码（安卓Bitmap.compress就是false）
 * outfileName_:压缩后的文件保存路径
 * return:1->成功  -1->失败  -2->bitmap的format不是ANDROID_BITMAP_FORMAT_RGBA_8888
 */
extern "C"
JNIEXPORT jint JNICALL
Java_com_jni_libjpeg_utils_CompressImageUtil_compressBitmap(JNIEnv *env, jclass type,
                                                            jobject bitmap, jint quality,
                                                            jstring outFileName_,
                                                            jboolean optimize,
                                                            jboolean recycle) {
    return compress_image(env,type,bitmap,quality,outFileName_,optimize,recycle);
}
