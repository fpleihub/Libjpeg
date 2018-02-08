package libjpeg.handpay.com.libjpeg;

import android.graphics.Bitmap;

import java.io.ByteArrayOutputStream;

/**
 * Created by fplei on 2018/1/17.
 * @author fplei
 */
public class ImageCompressHelper {

    public static Bitmap compress(){

        return null;
    }

    public static native boolean doCompress4File(Bitmap bitmap, int width, int height, String fileName, int quality);
    public static native Bitmap doCompress4Bitmap(Bitmap bitmap,int width, int height,int quality);
    public static native void doCompress4Byte(Bitmap bitmap, int width, int height, ByteArrayOutputStream byteArrayOutputStream, int quality);
    public static native Bitmap opt_big_or_small(Bitmap bitmap,float x_scale_rate,float y_scale_rate);
}
