package com.jni.libjpeg.utils;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * 图片压缩工具
 * Created by cj_28 on 2017/8/23.
 */

public class CompressImageUtil {

    public static final String TAG = CompressImageUtil.class.getSimpleName();

    static {
        System.loadLibrary("compress_image");
    }

    /**
     *
     * 压缩图片，压缩后会释放Bitmap对象
     *
     * @param bitmap
     * @param quality  0 > quality < 100
     * @param outFileName 压缩后的文件保存路径
     * @param optimize  true->使用哈夫曼编码，false->不适用哈夫曼编码（安卓Bitmap.compress就是false）
     * @param recycle  true->释放Bitmap资源，false->不释放Bitmap资源
     * @return 1->成功  -1->失败  -2->bitmap的format不是ANDROID_BITMAP_FORMAT_RGBA_8888
     */
    public static native int compressBitmap(Bitmap bitmap, int quality, String outFileName,
                                            boolean optimize, boolean recycle);

    /**
     * 获取Bitmap
     * @param pathName
     * @return
     */
    public static Bitmap decodeFile(String pathName) {
        int simpleWidth = 920;
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(pathName,options);

        int simpleSize = 1;
        if (simpleWidth < options.outWidth) {
            simpleSize = Math.round(options.outWidth * 1.0f / simpleWidth);
        }

        options.inSampleSize = simpleSize;

        options.inJustDecodeBounds = false;
        Bitmap bitmap = BitmapFactory.decodeFile(pathName, options);
        return bitmap;
    }

    /**
     *
     * @param path 要压缩的文件
     * @param parentPath 压缩后的文件保存的文件夹
     * @return 压缩后的文件路径
     */
    public static String compressPicture(String path, String parentPath){
        return compressPicture(path,parentPath,true);
    }

    /**
     *
     * @param path 要压缩的文件
     * @param parentPath 压缩后的文件保存的文件夹
     * @param recycle 是否使用c释放bitmap对象
     * @return 压缩后的文件路径
     */
    public static String compressPicture(String path, String parentPath,boolean recycle){
        String compressPath = null;
        if(path != null){
            File file = new File(path);
            if (file.exists()) {
                Bitmap bitmap = decodeFile(path);
                String filePath = FileUtil.createFilePath(parentPath, UUID.randomUUID() + getExtensionName(path,".png"));
                int quality = calculationQuality(path);
                int i = compressBitmap(bitmap, quality, filePath, true, recycle);
                if (!bitmap.isRecycled()) {
                    bitmap.recycle();
                }
                if (i == 1) {
                    compressPath = filePath;
                }else{
                    Log.e(TAG,"bitmap compress fail , file path = " + filePath);
                }
            }
        }
        return compressPath;
    }

    /**
     * 批量压缩
     * @param paths 要压缩的文件
     * @param parentPath 压缩后的文件保存的文件夹
     * @param recycle 是否使用c释放bitmap对象
     * @return
     */
    public static List<String> compressPictures(List<String> paths, String parentPath,boolean recycle){
        List<String> result = new ArrayList<>();
        if (paths != null && !paths.isEmpty()) {
            for (String path : paths) {
                String compressPath = compressPicture(path, parentPath, recycle);
                if(compressPath != null){
                    result.add(compressPath);
                }
            }
        }
        return result;
    }

    /**
     * 批量压缩
     * @param paths 要压缩的文件
     * @param parentPath 压缩后的文件保存的文件夹
     * @return
     */
    public static List<String> compressPictures(List<String> paths, String parentPath){
        return compressPictures(paths,parentPath,true);
    }

    public static int calculationQuality(String path){
        int resultSize = 100 * 1024;//最后压缩的小于100KB
        File file = new File(path);
        long length = file.length();
        float col =  (resultSize * 1000f) / length;
        int result = col >= 100 ? 100 : col < 1 ? 1 : (int)col;
        return result;
    }

    /**
     * 获取扩展名
     * @param filename
     * @param defalutExt
     * @return
     */
    public static String getExtensionName(String filename,String defalutExt) {
        if ((filename != null) && (filename.length() > 0)) {
            int dot = filename.lastIndexOf('.');
            if ((dot >-1) && (dot < (filename.length() - 1))) {
                return filename.substring(dot);
            }
        }
        return defalutExt;
    }

    public static int calculationQuality(long length){
        int resultSize = 100 * 1024;//最后压缩的小于100KB
        float col =  (resultSize * 1000f) / length;
        int result = col >= 100 ? 100 : col < 1 ? 1 : (int)col;
        return result;
    }

}
