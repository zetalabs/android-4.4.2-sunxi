/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HARDWARE_ITVDECODERSERVICE_H
#define ANDROID_HARDWARE_ITVDECODERSERVICE_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <gui/Surface.h>


namespace android {

class ITVDecoderService : public IInterface
{
public:
	enum {
		CONNECT = IBinder::FIRST_CALL_TRANSACTION,
		DISCONNECT,
		START_DECODER,
		STOP_DECODER,
		SET_COLOR,
		SET_SIZE,
		SET_PREVIEW_DISPLAY,
	};

public:
	DECLARE_META_INTERFACE(TVDecoderService);
	virtual status_t connect(int pid,int previewhw,int intf,int fmt,int chan) = 0;
	virtual status_t disconnect(int pid) = 0;
	virtual status_t startDecoder() = 0;
	virtual status_t stopDecoder() = 0;
	virtual status_t setColor(int luma,int contrast,int saturation,int hue) = 0;
	virtual status_t setSize(int x,int y,int w,int h) = 0;
	virtual status_t setPreviewDisplay(const sp<Surface>& surface) = 0;
};

// ----------------------------------------------------------------------------

class BnTVDecoderService: public BnInterface<ITVDecoderService>
{
public:
	virtual status_t onTransact( uint32_t code,const Parcel& data,Parcel* reply,uint32_t flags = 0);
};

}; // namespace android

#endif
