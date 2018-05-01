package com.jni.jnidemo;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;

import com.bumptech.glide.Glide;
import com.bumptech.glide.load.engine.DiskCacheStrategy;
import com.jni.jnidemo.utils.Util;

import java.io.File;

import uk.co.senab.photoview.PhotoView;

/**
 * Created by cj_28 on 2018/4/28.
 */

public class PhotoViewActivity extends AppCompatActivity {

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_NO_TITLE);//隐藏标题栏
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,WindowManager.LayoutParams.FLAG_FULLSCREEN);//隐藏状态栏
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_photo_view);
        String path = getIntent().getStringExtra("path");
        PhotoView photoView = (PhotoView) findViewById(R.id.image);
        Glide.with(this).load(path).skipMemoryCache(true)
                .diskCacheStrategy(DiskCacheStrategy.NONE).into(photoView);
        TextView size = (TextView) findViewById(R.id.size);
        size.setText(Util.getFormatSize(new File(path).length()));
    }
}
