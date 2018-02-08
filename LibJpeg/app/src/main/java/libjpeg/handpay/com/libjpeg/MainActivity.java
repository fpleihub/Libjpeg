package libjpeg.handpay.com.libjpeg;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.os.Environment;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;

import java.io.ByteArrayOutputStream;
import java.io.File;

public class MainActivity extends Activity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("libjpeg_utils");
    }
    ImageView imageView=null;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView=(ImageView)findViewById(R.id.image);
        Log.i("result",Environment.getExternalStorageDirectory().getAbsolutePath()+ File.separator+"crompress.jpg");
    }

    /**
     * 计算缩放比
     */
    private static int calculateInSampleSize(BitmapFactory.Options options, int reqWidth, int reqHeight) {
        final int height = options.outHeight;
        final int width = options.outWidth;

        int inSampleSize = 1;
        if (height > reqHeight || width > reqWidth) {
            final int halfHeight = height / 2;
            final int halfWidth = width / 2;
            while ((halfHeight / inSampleSize) > reqHeight && (halfWidth / inSampleSize) > reqWidth) {
                inSampleSize *= 2;
            }
        }
        Log.i("cpp_result","calculateInSampleSize->inSampleSize="+inSampleSize+",height="+height+",width="+width);
        return inSampleSize;
    }

    public void compressBitmap(String filePath){
        BitmapFactory.Options options = new BitmapFactory.Options();
        // 只解析图片边沿，获取宽高
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(filePath, options);
        //采样率, 数值越高，图片像素越低
        options.inSampleSize = calculateInSampleSize(options,960,1280);
        // 完整解析图片返回bitmap
        options.inJustDecodeBounds = false;
        Bitmap bitmap = BitmapFactory.decodeFile(filePath, options);
        bitmap=compressBitmapSize(bitmap,960,1280);
        String image_path=Environment.getExternalStorageDirectory().getAbsolutePath()+ File.separator+"crompress.jpg";
        boolean flag= ImageCompressHelper.doCompress4File(bitmap,-1,-1,image_path,30);
        Log.i("cpp_result","flag="+flag);
        bitmap=BitmapFactory.decodeFile(image_path);
        imageView.setImageBitmap(bitmap);
    }

    /**
     * 尺寸压缩，通过缩放图片像素来减少图片占用内存大小
     * @param bmp
     * @param reqWidth 期望压缩到指定宽,需要等比缩放，传入-1即可
     * @param reqHeight 期望压缩到指定高
     * @return
     */
    public Bitmap compressBitmapSize(Bitmap bmp,int reqWidth,int reqHeight){
        // 尺寸压缩倍数,值越大，图片尺寸越小
        int ratio=getRatioSize(bmp.getWidth(),bmp.getHeight());
        Log.i("cpp_result","ratio="+ratio);
        if(reqWidth<=0){
            reqWidth=bmp.getWidth() / ratio;
        }
        if(reqHeight<=0){
            reqHeight=bmp.getHeight() / ratio;
        }
        // 压缩Bitmap到对应尺寸
        Bitmap result = Bitmap.createBitmap(reqWidth, reqHeight, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(result);
        Rect rect = new Rect(0, 0, reqWidth, reqHeight);
        canvas.drawBitmap(bmp, null, rect, null);
        try {
            if (!bmp.isRecycled()) {
                bmp.recycle();
            }
        }catch (Exception e){
            Log.e("cpp_result","compressBitmapSize recycle error,other："+e.toString());
        }
        return result;
    }

    public static int getRatioSize(int bitWidth, int bitHeight) {
        Log.i("cpp_result","bitWidth="+bitWidth+",bitHeight="+bitHeight);
        // 图片最大分辨率
        int imageHeight = 1280;
        int imageWidth = 960;
        // 缩放比
        int ratio = 1;
        // 缩放比,由于是固定比例缩放，只用高或者宽其中一个数据进行计算即可
        if (bitWidth > bitHeight && bitWidth > imageWidth) {
            // 如果图片宽度比高度大,以宽度为基准
            ratio = (int)Math.ceil((double)bitWidth / (double) imageWidth);
        } else if (bitWidth < bitHeight && bitHeight > imageHeight) {
            // 如果图片高度比宽度大，以高度为基准
            ratio = (int)Math.ceil((double)bitHeight / (double) imageHeight);
        }
        // 最小比率为1
        if (ratio <= 0) {
            ratio = 1;
        }
        return ratio;
    }

    public void crom(View v){
        long time_before=System.currentTimeMillis();
        compressBitmap(Environment.getExternalStorageDirectory().getAbsolutePath()+ File.separator+"image3.jpg");
        long time_after=System.currentTimeMillis();
        Log.i("cpp_result","cost time:"+(time_after-time_before));
    }

    public void scale_small(View v){
        Bitmap bitmap=BitmapFactory.decodeResource(getResources(),R.drawable.wellcome2);
        Log.i("cpp_result","orig "+bitmap.getWidth()+","+bitmap.getHeight());
        try {
            Bitmap newBitmap = ImageCompressHelper.opt_big_or_small(bitmap, 0.5f, 0.5f);
            if (newBitmap != null) {
                Log.i("cpp_result",newBitmap.getWidth()+","+newBitmap.getHeight());
                imageView.setImageBitmap(newBitmap);
            }else {
                Log.i("cpp_result","newBitmap is null");
            }
        }catch (Exception e){
            e.printStackTrace();
        }
    }

    public void scale_big(View v){
        Bitmap bitmap=BitmapFactory.decodeResource(getResources(),R.drawable.wellcome2);
        try {
            Bitmap newBitmap = ImageCompressHelper.opt_big_or_small(bitmap, 1.6f, 1.6f);
            if (newBitmap != null) {
                Log.i("cpp_result",newBitmap.getWidth()+","+newBitmap.getHeight());
                imageView.setImageBitmap(newBitmap);
            }else {
                Log.i("cpp_result","newBitmap is null");
                Toast.makeText(this,"scale_big->newBitmap is null",Toast.LENGTH_LONG);
            }
        }catch (Exception e){
            e.printStackTrace();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        System.runFinalizersOnExit(true);
    }
}
