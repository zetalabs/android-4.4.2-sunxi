#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include "../include/ITVDecoderService.h"

namespace android {

class BpTVDecoderService: public BpInterface<ITVDecoderService>
{
public:
	BpTVDecoderService(const sp<IBinder>& impl)
	: BpInterface<ITVDecoderService>(impl)
	{
	}

	virtual status_t connect(int pid,int previewhw,int intf,int fmt,int chan)
	{
		Parcel data, reply;
		data.writeInterfaceToken(ITVDecoderService::getInterfaceDescriptor());
		data.writeInt32(pid);
		data.writeInt32(previewhw);
		data.writeInt32(intf);
		data.writeInt32(fmt);
		data.writeInt32(chan);
		remote()->transact(BnTVDecoderService::CONNECT, data, &reply);
		return reply.readInt32();
	}
	virtual status_t disconnect(int pid)
	{
		Parcel data, reply;
		data.writeInterfaceToken(ITVDecoderService::getInterfaceDescriptor());
		data.writeInt32(pid);
		remote()->transact(BnTVDecoderService::DISCONNECT, data, &reply);
		return reply.readInt32();
	}
	virtual status_t startDecoder()
	{
		Parcel data, reply;
		data.writeInterfaceToken(ITVDecoderService::getInterfaceDescriptor());
		remote()->transact(BnTVDecoderService::START_DECODER, data, &reply);
		return reply.readInt32();
	}
	virtual status_t stopDecoder()
	{
		Parcel data, reply;
		data.writeInterfaceToken(ITVDecoderService::getInterfaceDescriptor());
		remote()->transact(BnTVDecoderService::STOP_DECODER, data, &reply);
		return reply.readInt32();
	}
	virtual status_t setColor(int luma,int contrast,int saturation,int hue)
	{
		Parcel data, reply;
		data.writeInterfaceToken(ITVDecoderService::getInterfaceDescriptor());
		data.writeInt32(luma);
		data.writeInt32(contrast);
		data.writeInt32(saturation);
		data.writeInt32(hue);
		remote()->transact(BnTVDecoderService::SET_COLOR, data, &reply);
		return reply.readInt32();
	}
	virtual status_t setSize(int x,int y,int w,int h)
	{
		Parcel data, reply;
		data.writeInterfaceToken(ITVDecoderService::getInterfaceDescriptor());
		data.writeInt32(x);
		data.writeInt32(y);
		data.writeInt32(w);
		data.writeInt32(h);
		remote()->transact(BnTVDecoderService::SET_SIZE, data, &reply);
		return reply.readInt32();
	}
	virtual status_t setPreviewDisplay(const sp<IGraphicBufferProducer>& bufferProducer)
	{
		Parcel data, reply;
		data.writeInterfaceToken(ITVDecoderService::getInterfaceDescriptor());
		sp<IBinder> b(bufferProducer->asBinder());
		data.writeStrongBinder(b);
		remote()->transact(SET_PREVIEW_DISPLAY, data, &reply);
		return reply.readInt32();
	}
};

IMPLEMENT_META_INTERFACE(TVDecoderService, "android.hardware.ITVDecoderService");

// ----------------------------------------------------------------------

status_t BnTVDecoderService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
	switch(code)
	{
		case CONNECT: {
			CHECK_INTERFACE(ITVDecoderService, data, reply);
			int pid         = data.readInt32();
			int previewhw   = data.readInt32();
			int intf        = data.readInt32();
			int fmt         = data.readInt32();
			int chan        = data.readInt32();
			reply->writeInt32(connect(pid,previewhw,intf,fmt,chan));
			return NO_ERROR;
			}break;
		case DISCONNECT: {
			CHECK_INTERFACE(ITVDecoderService, data, reply);
			reply->writeInt32(disconnect(data.readInt32()));
			return NO_ERROR;
			}break;
		case START_DECODER: {
			CHECK_INTERFACE(ITVDecoderService, data, reply);
			reply->writeInt32(startDecoder());
			return NO_ERROR;
			}break;
		case STOP_DECODER: {
			CHECK_INTERFACE(ITVDecoderService, data, reply);
			reply->writeInt32(stopDecoder());
			return NO_ERROR;
			}break;
		case SET_COLOR: {
			CHECK_INTERFACE(ITVDecoderService, data, reply);
			int luma = data.readInt32();
			int contrast = data.readInt32();
			int saturation = data.readInt32();
			int hue = data.readInt32();
			reply->writeInt32(setColor(luma,contrast,saturation,hue));
			return NO_ERROR;
			}break;
		case SET_SIZE: {
			CHECK_INTERFACE(ITVDecoderService, data, reply);
			int x = data.readInt32();
			int y = data.readInt32();
			int w = data.readInt32();
			int h = data.readInt32();
			reply->writeInt32(setSize(x,y,w,h));
			return NO_ERROR;
			}break;
		case SET_PREVIEW_DISPLAY: {
			CHECK_INTERFACE(ITVDecoderService, data, reply);
#if 0
			sp<Surface> surface = Surface::readFromParcel(data);
			reply->writeInt32(setPreviewDisplay(surface));
#else
			sp<IGraphicBufferProducer> st =
				interface_cast<IGraphicBufferProducer>(data.readStrongBinder());
			reply->writeInt32(setPreviewDisplay(st));
#endif
			return NO_ERROR;
			}break;
		default:
			return BBinder::onTransact(code, data, reply, flags);
	}
}

// ----------------------------------------------------------------------------

}; // namespace android
