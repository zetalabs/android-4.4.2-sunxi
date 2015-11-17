package softwinner.tv;

import android.view.Surface;


public class TVDecoder {
	public TVDecoder()
	{}
	static {
		try {
			System.loadLibrary("tvdecoder_jni");
		} catch (UnsatisfiedLinkError ule) {
			System.err.println("WARNING: Could not load tvdecoder_jni library!");
		}
	}
	public native int connect(int previewhw,int jinterface,int jformat,int jchannel);
	public native int disconnect();
	public native int startDecoder();
	public native int stopDecoder();
	public native int setColor(int luma,int contrast,int saturation,int hue);
	public native int setSize(int x,int y,int w,int h);
	public native int setPreviewDisplay(IGraphicBufferProducer bufferProducer);
}
