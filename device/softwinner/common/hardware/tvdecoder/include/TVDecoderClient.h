#ifndef ANDROID_HARDWARE_TVDECODERCLIENT_H
#define ANDROID_HARDWARE_TVDECODERCLIENT_H

//#include <gui/SurfaceTextureClient.h>
#include <gui/Surface.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
//#include <gui/ISurface.h>
#include <ui/FramebufferNativeWindow.h>
#include <ui/GraphicBuffer.h>
#include <binder/MemoryHeapBase.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <utils/Log.h>
#include <utils/Vector.h>
#include <utils/threads.h>

#include "ITVDecoderService.h"

namespace android {
class TVDecoderClient
{
public:
								TVDecoderClient();
	virtual                     ~TVDecoderClient();
	virtual int32_t             connect(int intf,int previewhw,int fmt,int chan);
	virtual int32_t             disconnect();
	virtual int32_t             startDecoder();
	virtual int32_t             stopDecoder();
	virtual int32_t             setColor(int luma,int contrast,int saturation,int hue);
	virtual int32_t             setSize(int x,int y,int w,int h);
	virtual int32_t             setPreviewDisplay(const sp<IGraphicBufferProducer>& bufferProducer);
private:
	sp<ITVDecoderService>       mTVDecoderService;
};
};  // namespace android
#endif // ANDROID_HARDWARE_TVDECODERCLIENT_H

