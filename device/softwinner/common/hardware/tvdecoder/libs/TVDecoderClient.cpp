#define LOG_NDEBUG 1
#define LOG_TAG "TVDecoderClient"

#include "../include/TVDecoderClient.h"



namespace android {
static int getCallingPid() {
    return IPCThreadState::self()->getCallingPid();
}

static int getCallingUid() {
    return IPCThreadState::self()->getCallingUid();
}

TVDecoderClient::TVDecoderClient()
{
	sp<IServiceManager> sm = defaultServiceManager();
	sp<IBinder> binder = sm->getService(String16("media.tvd"));
	mTVDecoderService = interface_cast<ITVDecoderService>(binder);
}

TVDecoderClient::~TVDecoderClient()
{
	mTVDecoderService = NULL;
}

int32_t TVDecoderClient::connect(int previewhw,int intf,int fmt,int chan)
{
	return mTVDecoderService->connect(getCallingPid(),previewhw,intf,fmt,chan);
}

int32_t TVDecoderClient::disconnect()
{
        return mTVDecoderService->disconnect(getCallingPid());
}

int32_t TVDecoderClient::startDecoder()
{
	return mTVDecoderService->startDecoder();
}

int32_t TVDecoderClient::stopDecoder()
{
	return mTVDecoderService->stopDecoder();
}

int32_t TVDecoderClient::setColor(int luma,int contrast,int saturation,int hue)
{
	return mTVDecoderService->setColor(luma,contrast,saturation,hue);
}

int32_t TVDecoderClient::setSize(int x,int y,int w,int h)
{
	return mTVDecoderService->setSize(x,y,w,h);
}

int32_t TVDecoderClient::setPreviewDisplay(const sp<IGraphicBufferProducer>& bufferProducer)
{
	ALOGV("setPreviewDisplay(%p)", bufferProducer.get());
	sp <ITVDecoderService> tvd = mTVDecoderService;
	if (tvd == 0) return NO_INIT;
	ALOGD_IF(bufferProducer == 0, "app passed NULL surface");
	return tvd->setPreviewDisplay(bufferProducer);
}

}; // namespace android

