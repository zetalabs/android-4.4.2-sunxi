#define LOG_NDEBUG 1
#define LOG_TAG "TVDecoderService"

#include <jni.h>
#include <utils/Log.h>
//#include <gui/SurfaceTextureClient.h>
#include <gui/Surface.h>
#include <binder/IPCThreadState.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cutils/atomic.h>
#include <sys/types.h>
#include <pthread.h>
#include <ui/PixelFormat.h>
#include <ui/FramebufferNativeWindow.h>
#include <fcntl.h>
#include <sys/mman.h> 
#include <utils/threads.h>
#include <hardware/hwcomposer.h>
#include <system/window.h>
#include <ui/GraphicBufferMapper.h>
#include <binder/IServiceManager.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <binder/IBinder.h>
#include <hardware/hardware.h>
#include <utils/Errors.h>
#include <assert.h>

#include <g2d_driver.h>
#include "TVDecoderService.h"

#define CHECK assert
#define CHECK_EQ(a,b) CHECK((a)==(b))

#define TVD_PREVIEW_HW 1
#define TVD_NATIVE 0

#ifndef container_of
#define container_of(ptr, type, member) ({                      \
        const typeof(((type *) 0)->member) *__mptr = (ptr);     \
        (type *) ((char *) __mptr - (char *)(&((type *)0)->member)); })
#endif

namespace android {
static int getCallingPid() {
    return IPCThreadState::self()->getCallingPid();
}

static int getCallingUid() {
    return IPCThreadState::self()->getCallingUid();
}
static TVDecoderService *gTVDecoderService;

TVDecoderService::TVDecoderService():
    mFirstFrame(true),
    mVideoWidth(0),
    mVideoHeight(0),
    mVideoFormat(0),
    mV4l2Handle(0),
    mBufferCnt(5),
    mTVDecoderPreviewThread(0),
    mFirstTime(true),
    mTVDecoderPreviewThreadStarted(false),
    mMirror(0),
    mLuma(50),
    mContrast(50),
    mSaturation(50),
    mHue(50)
{
    ALOGD("TVDecoderService started (pid=%d)", getpid());
    gTVDecoderService = this;
    memset(&mMapMem, 0, sizeof(v4l2_mem_map_t));
    initDefaultParameters();
}

void TVDecoderService::onFirstRef()
{
    mSystem = TVD_NTSC;
    mOpened = 0;
    mPreviewWindow = 0;
    mConnectFinished = 0;
    mConnecting = 0;
    mPreviewHardware = 0;
    BnTVDecoderService::onFirstRef();
}
TVDecoderService::~TVDecoderService()
{
    gTVDecoderService = NULL;
}

status_t TVDecoderService::connect(int pid,int previewhw,int intf,int fmt,int chan)
{
    ALOGV("####################TVDecoderService::connect pid %d\n",pid);
    int32_t ret = OK;
    mLock.lock();
    if(mConnecting)
    {
        ALOGV("Decoder connecting");
        mConnecting = 0;
        mLock.unlock();
        return OK;
    }
    mConnecting = 1;
    if(pid == mPid)
    {
        ALOGV("Decoder already connect");
        mConnecting = 0;
        mLock.unlock();
        return OK;
    }
    mPid = pid;
    mPreviewHardware = previewhw;
    if(mOpened == 0)
    {
        ALOGV("####################TVDecoderService::not need disconnect pid %d\n",gettid());
        ret = openCameraDev();
        if (ret != OK)
        {
            ALOGE("ERROR: openCameraDev error, line: %d", __LINE__);
            mConnecting = 0;
            mLock.unlock();
            return UNKNOWN_ERROR;
        }
    }
    else
    {
    ALOGV("####################TVDecoderService::need disconnect pid %d\n",gettid());
        if (mTVDecoderPreviewThreadStarted)
        {
            if (mTVDecoderPreviewThread != 0)
            {
                ALOGV("wait tvdecoder thread exit");
                mTVDecoderPreviewThread->requestExitAndWait();
                ALOGV("tvdecoder thread exit ok");		
                mTVDecoderPreviewThread.clear();
                mTVDecoderPreviewThread = 0;
            }
            ret = v4l2StopStreaming();
            if (ret != OK)
            {
                ALOGE("ERROR: v4l2StopStreaming error, line: %d", __LINE__);
                mConnecting = 0;
                mLock.unlock();
                return UNKNOWN_ERROR;
            }
        }
        else
        {
            ret = v4l2StartStreaming();
            if (ret != OK)
            {
                ALOGE("ERROR: v4l2SetVideoParams error, line: %d", __LINE__);
                mConnecting = 0;
                mLock.unlock();
                return UNKNOWN_ERROR;
            }
            ret = v4l2StopStreaming();
            if (ret != OK)
            {
                ALOGE("ERROR: v4l2StopStreaming error, line: %d", __LINE__);
                mConnecting = 0;
                mLock.unlock();
                return UNKNOWN_ERROR;
            }
        }
        ret = v4l2UnmapBuf();
        if (ret != OK)
        {
            ALOGE("ERROR: v4l2UnmapBuf error, line: %d", __LINE__);
            mConnecting = 0;
            mLock.unlock();
            return UNKNOWN_ERROR;
        }
        closeCameraDev();
        mFirstFrame = true;
        mTVDecoderPreviewThreadStarted = false;
        mConnectFinished = 0;
        ret = openCameraDev();
        if (ret != OK)
        {
            ALOGE("ERROR: openCameraDev error, line: %d", __LINE__);
            mConnecting = 0;
            mLock.unlock();
            return UNKNOWN_ERROR;
        }
    }
    mInterface      = intf;
    mFormat         = fmt;
    mChannel        = chan;
    ret = v4l2SetVideoParams();
    if (ret != OK)
    {
        ALOGE("ERROR: v4l2SetVideoParams error, line: %d", __LINE__);
        mConnecting = 0;
        mLock.unlock();
        return UNKNOWN_ERROR;
    }
    ret = v4l2ReqBufs();
    if (ret != OK)
    {
        ALOGE("ERROR: v4l2ReqBufs error, line: %d", __LINE__);
        mConnecting = 0;
        mLock.unlock();
        return UNKNOWN_ERROR;
    }
    ret = v4l2QueryBuf();
    if (ret != OK)
    {
        ALOGE("ERROR: v4l2QueryBuf error, line: %d", __LINE__);
        mConnecting = 0;
        mLock.unlock();
        return UNKNOWN_ERROR;
    }
    mConnectFinished = 1;
    mConnecting = 0;
    mLock.unlock();
    return ret;
}

status_t TVDecoderService::disconnect(int pid)
{
    ALOGV("####################TVDecoderService::disconnect pid %d\n",pid);
    int32_t ret;
    mLock.lock();
    if(mConnectFinished == 0)
    {
        ALOGV("already disconnect,or connect have not finished");
        mLock.unlock();
        return OK;
    }
    if(pid != mPid)
    {
        ALOGV("other process connected");
        mLock.unlock();
        return OK;
    }

    if (mTVDecoderPreviewThreadStarted)
    {
        if (mTVDecoderPreviewThread != 0)
        {
            ALOGV("wait tvdecoder thread exit");
            mTVDecoderPreviewThread->requestExitAndWait();
            ALOGV("tvdecoder thread exit ok");		
            mTVDecoderPreviewThread.clear();
            mTVDecoderPreviewThread = 0;
        }
        ret = v4l2StopStreaming();
        if (ret != OK)
        {
            ALOGE("ERROR: v4l2StopStreaming error, line: %d", __LINE__);
            mLock.unlock();
            return UNKNOWN_ERROR;
        }
    }
    else
    {
        ret = v4l2StartStreaming();
        if (ret != OK)
        {
            ALOGE("ERROR: v4l2SetVideoParams error, line: %d", __LINE__);
            return UNKNOWN_ERROR;
        }
        ret = v4l2StopStreaming();
        if (ret != OK)
        {
            ALOGE("ERROR: v4l2StopStreaming error, line: %d", __LINE__);
            return UNKNOWN_ERROR;
        }
    }
    if(mPreviewHardware)
    {
        mPreviewWindow.get()->perform(mPreviewWindow.get(), NATIVE_WINDOW_SETPARAMETER, HWC_LAYER_SHOW, 0);
    }
    ret = v4l2UnmapBuf();
    if (ret != OK)
    {
        ALOGE("ERROR: v4l2UnmapBuf error, line: %d", __LINE__);
        mLock.unlock();
        return UNKNOWN_ERROR;
    }
    closeCameraDev();
    mFirstFrame = true;
    mTVDecoderPreviewThreadStarted = false;
    mConnectFinished = 0;
    mPid = 0;
    mLock.unlock();
    return OK;
}

status_t TVDecoderService::startDecoder()
{
    ALOGV("####################TVDecoderService::start_decoder \n");
    int32_t ret;
    if(mConnectFinished == 0)
    {
        ALOGE("Decoder have not connected,startDecoder after connect");
        return -1;
    }
    if(mPreviewHardware)
    {
        mPreviewWindow.get()->perform(mPreviewWindow.get(), NATIVE_WINDOW_SETPARAMETER, HWC_LAYER_SHOW, 1);
    }
    if (mTVDecoderPreviewThreadStarted)
    {
        ALOGW("TVDecoderService::TVDecoderPreviewThread: state already started");
        return OK;
    }
    if(mPreviewWindow == NULL)
    {
        ALOGE("TVDecoderService : mPreviewWindow is null");
        return UNKNOWN_ERROR;
    }
    ret = v4l2StartStreaming();
    if (ret != OK)
    {
        ALOGE("ERROR: v4l2SetVideoParams error, line: %d", __LINE__);
        return UNKNOWN_ERROR;
    }
    mFirstTime = true;
    mTVDecoderPreviewThread = new TVDecoderPreviewThread(this);
    mTVDecoderPreviewThreadStarted = true;
    return OK;
}

status_t TVDecoderService::stopDecoder()
{
    ALOGV("####################TVDecoderService::stop_decoder \n");
    int32_t ret;
    if(mPreviewHardware)
    {
        mPreviewWindow.get()->perform(mPreviewWindow.get(), NATIVE_WINDOW_SETPARAMETER, HWC_LAYER_SHOW, 0);
    }
    if (!mTVDecoderPreviewThreadStarted)
    {
        ALOGW("TVDecoderService::stopTVDecoder: state already stopped");
        return OK;
    }
    if (mTVDecoderPreviewThread != 0)
    {
        ALOGV("wait tvdecoder thread exit");
        mTVDecoderPreviewThread->requestExitAndWait();
        ALOGV("tvdecoder thread exit ok");		
        mTVDecoderPreviewThread.clear();
        mTVDecoderPreviewThread = 0;
    }
    ret = v4l2StopStreaming();
    if (ret != OK)
    {
        ALOGE("ERROR: v4l2StopStreaming error, line: %d", __LINE__);
        return UNKNOWN_ERROR;
    }
    mFirstFrame = true;
    mTVDecoderPreviewThreadStarted = false;
    return OK;
}

status_t TVDecoderService::setSize(int x,int y,int w,int h)
{
    ALOGV("####################TVDecoderService::set_size %d  %d  %d  %d \n",x,y,w,h);
    mScreenX = x;
    mScreenY = y;
    mScreenWidth = w;
    mScreenHeight = h;
    return OK;
}

status_t TVDecoderService::setPreviewDisplay(const sp<IGraphicBufferProducer>& bufferProducer)
{
    int ret = OK;
#if 0
    sp<IBinder> binder(surface != 0 ? surface->asBinder() : 0);
    sp<ANativeWindow> window(surface);
#else
    sp<IBinder> binder;
    sp<ANativeWindow> window;

    ALOGD("setPreviewTarget(%p) (pid %d)", bufferProducer.get(),
        getCallingPid());

    if (bufferProducer != 0) {
        binder = bufferProducer->asBinder();
        // Using controlledByApp flag to ensure that the buffer queue remains in
        // async mode for the old camera API, where many applications depend
        // on that behavior.
        window = new Surface(bufferProducer, /*controlledByApp*/ true);
    }
#endif
    mPreviewWindow = window;
#if TVD_NATIVE
    mPreviewWindow = getNativeWindow();
    native_window_api_connect(mPreviewWindow.get(), NATIVE_WINDOW_API_CAMERA_HW);
    native_window_set_scaling_mode(mPreviewWindow.get(),NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    native_window_set_buffers_transform(mPreviewWindow.get(), 0);
#else
    if(mPreviewHardware)
    {
        if (window != 0)
        {
            ret = native_window_api_connect(mPreviewWindow.get(), NATIVE_WINDOW_API_CAMERA_HW);
            if (ret != NO_ERROR)
            {
                ALOGE("native_window_api_connect failed: %s (%d), api: %d", strerror(-ret),ret, NATIVE_WINDOW_API_CAMERA_HW);
                return ret;
            }
            native_window_set_scaling_mode(mPreviewWindow.get(),NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
            native_window_set_buffers_transform(mPreviewWindow.get(), 0);
            ret = OK;
        }
        else
        {
            ALOGE("xxxxxxxxxxxxxxxxxxxxxxxx Window is NULL");
            ret = BAD_VALUE;
        }
    }
    else
    {
        if (window != 0)
        {
            ret = native_window_api_connect(mPreviewWindow.get(), NATIVE_WINDOW_API_CAMERA);
            if (ret != NO_ERROR)
            {
                ALOGE("native_window_api_connect failed: %s (%d), api: %d", strerror(-ret),ret, NATIVE_WINDOW_API_CAMERA);
                return ret;
            }
            native_window_set_scaling_mode(mPreviewWindow.get(),NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
            native_window_set_buffers_transform(mPreviewWindow.get(), 0);
            ret = OK;
        }
        else
        {
            ALOGE("xxxxxxxxxxxxxxxxxxxxxxxx Window is NULL");
            ret = BAD_VALUE;
        }
    }
#endif
    return ret;
}

status_t TVDecoderService::setColor(int luma,int contrast,int saturation,int hue)
{
    mLuma = luma;
    mContrast = contrast;
    mSaturation = saturation;
    mHue = hue;
    v4l2SetColor();
    return OK;
}

sp<ANativeWindow> TVDecoderService::getNativeWindow()
{
#if 0
    android::ProcessState::self()->startThreadPool();
#endif
    
    sp<SurfaceComposerClient> composerClient = new SurfaceComposerClient;
    CHECK_EQ(composerClient->initCheck(), (status_t)OK);

    sp<SurfaceControl> control = composerClient->createSurface(String8("tvd Surface"),
                                                               800,//mScreenWidth,
                                                               430,//mScreenHeight,
                                                               PIXEL_FORMAT_RGBA_8888,
                                                               0);
    CHECK(control != NULL);
    CHECK(control->isValid());
    SurfaceComposerClient::openGlobalTransaction();
    control->setPosition(mScreenX,mScreenY);
    CHECK_EQ(control->setLayer(10000), (status_t)OK);
    CHECK_EQ(control->show(), (status_t)OK);
    control->setAlpha(1.0);
    SurfaceComposerClient::closeGlobalTransaction();
    sp<Surface> surface = control->getSurface();
    CHECK(surface != NULL);
    mSurface = surface;
    mComposerClient = composerClient;
    mControl = control;
    // composerClient->dispose() at exit
    sp<ANativeWindow> window(surface);
    return window;
}
void TVDecoderService::initDefaultParameters(void)
{
    if(mMirror)
    {
        mG2DHandle = open("/dev/g2d", O_RDWR, 0);
        if (mG2DHandle < 0)
        {
            ALOGE("open /dev/g2d failed");
        }
        ALOGV("open /dev/g2d OK");

        int ret = cedarx_hardware_init(2);
        if (ret < 0)
        {
            ALOGE("cedarx_hardware_init failed");
        }
        ALOGV("cedarx_hardware_init ok");
    }
}

status_t TVDecoderService::tvDecoderPreviewThread()
{
    int ret = UNKNOWN_ERROR;
    int system;
    ret = getSystem(&system);
    if(ret == OK)
    {
        if(mSystem != system)
        {
            if(mPreviewHardware)
            {
                mPreviewWindow.get()->perform(mPreviewWindow.get(), NATIVE_WINDOW_SETPARAMETER, HWC_LAYER_SHOW, 0);
            }
            mSystem = system;
            mFirstTime = true;
            resetTVDecoder();
        }
    }

    ret = v4l2WaitCamerReady();
    if (ret != 0)
    {
        ALOGE("wait time out");
        return __LINE__;
    }
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(v4l2_buffer));
    ret = getPreviewFrame(&buf);
    if (ret != OK)
    {
        usleep(10000);
        return ret;
    }

    if(mPreviewHardware)
    {
        if(mFirstTime == true)
        {
            if(mMirror)
            {
                mFormat = TVD_PL_YUV420;
            }
            if(mFormat==TVD_MB_YUV420)
            {
                mBuffes_Geometry_Format = HWC_FORMAT_MBYUV420;
            }
            else
            {
                mBuffes_Geometry_Format = HWC_FORMAT_DEFAULT;
            }
            ret = native_window_set_buffers_geometryex(	mPreviewWindow.get() ,
                                                        mVideoWidth,
                                                        mVideoHeight,
                                                        mBuffes_Geometry_Format,
                                                        0);
            if (ret != NO_ERROR) 
            {
                ALOGE("%s: Error in set_buffers_geometry %d -> %s",__FUNCTION__, -ret, strerror(-ret));
                return ret;
            }
            ret=mPreviewWindow.get()->perform(mPreviewWindow.get(), NATIVE_WINDOW_SETPARAMETER, HWC_LAYER_SHOW, 1);
            if (ret != OK)
            {
                ALOGE("NATIVE_WINDOW_SETPARAMETER HWC_LAYER_SHOW failed");
                return ret;
            }
            mFirstTime = false;
        }
        libhwclayerpara_t overlay_para;
        if(mMirror)
        {
            overlay_para.bProgressiveSrc    = 0;
            overlay_para.bTopFieldFirst     = 1;
            overlay_para.top_y 		= (unsigned int)mVideoBuffer.buf_phy_addr[buf.index];
            overlay_para.top_c 		= (unsigned int)mVideoBuffer.buf_phy_addr[buf.index]+mVideoWidth*mVideoHeight;
            overlay_para.bottom_y 	        = 0;
            overlay_para.bottom_c 	        = 0;
            overlay_para.number 	        = 0;
        }
        else
        {
            overlay_para.bProgressiveSrc    = 0;
            overlay_para.bTopFieldFirst     = 1;
            overlay_para.top_y 		= (unsigned int)buf.m.offset;
            overlay_para.top_c 		= (unsigned int)buf.m.offset+mVideoWidth*mVideoHeight;
            overlay_para.bottom_y 	        = 0;
            overlay_para.bottom_c 	        = 0;
            overlay_para.number 	        = 0;
        }
        ret = mPreviewWindow.get()->perform(mPreviewWindow.get(), NATIVE_WINDOW_SETPARAMETER, HWC_LAYER_SETFRAMEPARA, (uint32_t)&overlay_para);
        if (ret != OK)
        {
            ALOGE("NATIVE_WINDOW_SETPARAMETER failed");
            return ret;
        }
    }
    else
    {
        if(mFirstTime == true)
        {
            mFirstTime = false;
            ret = native_window_set_buffers_geometry(       mPreviewWindow.get(),
            mVideoWidth,
            mVideoHeight,
            HAL_PIXEL_FORMAT_YCrCb_420_SP);
            if (ret != NO_ERROR)
            {
                ALOGE("%s: Error in set_buffers_geometry %d -> %s",__FUNCTION__, -ret, strerror(-ret));
                return ret;
            }
            
            ret = native_window_set_buffer_count(mPreviewWindow.get(),3);
            if (ret != 0)
            {
                ALOGE("native_window_set_buffer_count failed: %s (%d)", strerror(-ret), -ret);
                if ( ENODEV == ret )
                {
                    ALOGE("Preview surface abandoned!");
                }
                return ret;
            }
        }

        buffer_handle_t* buffer = NULL;
        int stride = 0;
        ANativeWindowBuffer* anb;
        ret = mPreviewWindow.get()->dequeueBuffer_DEPRECATED(mPreviewWindow.get(),&anb);
        if (!ret)
        {
            buffer = &anb->handle;
            stride = anb->stride;
        }

        if (ret != NO_ERROR || buffer == NULL)
        {
            ALOGE("%s: Unable to dequeue preview window buffer: %d -> %s",__FUNCTION__, -ret, strerror(-ret));
            return ret;
        }
        ret = mPreviewWindow.get()->lockBuffer_DEPRECATED(mPreviewWindow.get(),container_of(buffer, ANativeWindowBuffer, handle));
        if (ret != NO_ERROR)
        {
            ALOGE("%s: Unable to lock preview window buffer: %d -> %s",__FUNCTION__, -ret, strerror(-ret));
            //mPreviewWindow.get()->cancelBuffer(mPreviewWindow.get(),container_of(buffer, ANativeWindowBuffer, handle));
            return ret;
        }
        
        void* img = NULL;
        const Rect rect(mVideoWidth, mVideoHeight);
        GraphicBufferMapper& grbuffer_mapper(GraphicBufferMapper::get());
        ret = grbuffer_mapper.lock(*buffer, GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &img);
        if (ret != NO_ERROR)
        {
            ALOGE("%s: grbuffer_mapper.lock failure: %d -> %s",__FUNCTION__, ret, strerror(ret));
            //mPreviewWindow.get()->cancelBuffer(mPreviewWindow.get(),container_of(buffer, ANativeWindowBuffer, handle));
            return ret;
        }
        if(mMirror)
            memcpy((void*)img, (void*)mVideoBuffer.buf_vir_addr[buf.index], mVideoWidth*mVideoHeight*3/2);
        else
            memcpy((void*)img, (void*)mMapMem.mem[buf.index], mVideoWidth*mVideoHeight*3/2);
        if (ret == NO_ERROR)
        {
            mPreviewWindow.get()->queueBuffer_DEPRECATED(mPreviewWindow.get(),container_of(buffer, ANativeWindowBuffer, handle));
        }
        else
        {
            ALOGE("%s: Unable to obtain preview frame: %d", __FUNCTION__, ret);
            //mPreviewWindow.get()->cancelBuffer(mPreviewWindow.get(),container_of(buffer, ANativeWindowBuffer, handle));
        }
        grbuffer_mapper.unlock(*buffer);
    }
    releasePreviewFrame(buf.index);
    return OK;
}

status_t TVDecoderService::resetTVDecoder()
{
    int32_t ret;
    if(mConnectFinished == 0)
    {
        ALOGE("ERROR: resetTVDecoder err,decoder not connect");
        return UNKNOWN_ERROR;
    }
    ret = v4l2StopStreaming();
    if (ret != OK)
    {
        ALOGE("ERROR: v4l2StopStreaming error, line: %d", __LINE__);
        return UNKNOWN_ERROR;
    }

    ret = v4l2UnmapBuf();
    if (ret != OK)
    {
        ALOGE("ERROR: v4l2UnmapBuf error, line: %d", __LINE__);
        return UNKNOWN_ERROR;
    }

    ret = v4l2SetVideoParams();
    if (ret != OK)
    {
        ALOGE("ERROR: v4l2SetVideoParams error, line: %d", __LINE__);
        return UNKNOWN_ERROR;
    }
    ret = v4l2ReqBufs();
    if (ret != OK)
    {
        ALOGE("ERROR: v4l2ReqBufs error, line: %d", __LINE__);
        return UNKNOWN_ERROR;
    }
    ret = v4l2QueryBuf();
    if (ret != OK)
    {
        ALOGE("ERROR: v4l2QueryBuf error, line: %d", __LINE__);
        return UNKNOWN_ERROR;
    }
    ret = v4l2StartStreaming();
    if (ret != OK)
    {
        ALOGE("ERROR: v4l2SetVideoParams error, line: %d", __LINE__);
        return UNKNOWN_ERROR;
    }
    return ret;
}

int TVDecoderService::openCameraDev()
{
    mV4l2Handle = open("/dev/video10",O_RDWR|O_NONBLOCK,0);
    if (mV4l2Handle == -1)
    {
        ALOGE("ERROR opening V4L interface: %s", strerror(errno));
        return -1;
    }
    int ret = -1;
    struct v4l2_capability cap;
    ret = ioctl (mV4l2Handle, VIDIOC_QUERYCAP, &cap);
    if (ret < 0)
    {
        ALOGE("Error opening device: unable to query device.");
        return -1;
    }
    if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0)
    {
        ALOGE("Error opening device: video capture not supported.");
        return -1;
    }
    if ((cap.capabilities & V4L2_CAP_STREAMING) == 0)
    {
        ALOGE("Capture device does not support streaming i/o");
        return -1;
    }
    mOpened = 1;
    return OK;
}

void TVDecoderService::closeCameraDev()
{
    if (mV4l2Handle)
    {
        close(mV4l2Handle);
        mV4l2Handle = 0;
        mOpened = 0;
    }
}

int TVDecoderService::getSystem(int *system)
{
    int ret = OK;
    struct v4l2_format format;
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_PRIVATE;
    if (-1 == ioctl (mV4l2Handle, VIDIOC_G_FMT, &format))
    {
        ALOGE("VIDIOC_G_FMT error!  a\n");
        ret = -1;
        return ret;
    }
    switch(mChannel)
    {
        case TVD_CHANNEL_ONLY_1:
            if((format.fmt.raw_data[16] & 1) == 0)
            {
                ret = -1;
                return ret;
            }
            if((format.fmt.raw_data[16]&(1<<4))!=0)
            {
                *system = TVD_PAL;
            }
            else if((format.fmt.raw_data[16]&(1<<4))==0)
            {
                *system = TVD_NTSC;
            }
            ALOGV("format.fmt.raw_data[16] =0x%x",format.fmt.raw_data[16]);
            break;
        case TVD_CHANNEL_ONLY_2:
            if((format.fmt.raw_data[17] & 1) == 0)
            {
                ret = -1;
                return ret;
            }
            if((format.fmt.raw_data[17]&(1<<4))!=0)
            {
                *system = TVD_PAL;
            }
            else if((format.fmt.raw_data[17]&(1<<4))==0)
            {
                *system = TVD_NTSC;
            }
            ALOGV("format.fmt.raw_data[17] =0x%x",format.fmt.raw_data[17]);
            break;
        case TVD_CHANNEL_ONLY_3:
            if((format.fmt.raw_data[18] & 1) == 0)
            {
                ret = -1;
                return ret;
            }
            if((format.fmt.raw_data[18]&(1<<4))!=0)
            {
                *system = TVD_PAL;
            }
            else if((format.fmt.raw_data[18]&(1<<4))==0)
            {
                *system = TVD_NTSC;
            }
            ALOGV("format.fmt.raw_data[18] =0x%x",format.fmt.raw_data[18]);
            break;
        case TVD_CHANNEL_ONLY_4:
            if((format.fmt.raw_data[19] & 1) == 0)
            {
                ret = -1;
                return ret;
            }
            if((format.fmt.raw_data[19]&(1<<4))!=0)
            {
                *system = TVD_PAL;
            }
            else if((format.fmt.raw_data[19]&(1<<4))==0)
            {
                *system = TVD_NTSC;
            }
            ALOGV("format.fmt.raw_data[19] =0x%x",format.fmt.raw_data[19]);
            break;
        case TVD_CHANNEL_ALL_2x2:
        case TVD_CHANNEL_ALL_1x4:
        case TVD_CHANNEL_ALL_4x1:
	    if (((format.fmt.raw_data[16] & 1) || (format.fmt.raw_data[17] & 1)
				    || (format.fmt.raw_data[18] & 1) || (format.fmt.raw_data[19] & 1)) == 0)
	    {
		    ret = -1;
		    return ret; 
	    }
	    if((format.fmt.raw_data[16]&(1<<4)) !=0 )
	    {
		    *system = TVD_PAL;
	    }
	    else if((format.fmt.raw_data[16]&(1<<4)) == 0)
	    {
		    *system = TVD_NTSC;  
	    }
	    break;
	case TVD_CHANNEL_ALL_1x2:
	case TVD_CHANNEL_ALL_2x1:
	    if (((format.fmt.raw_data[16] & 1) || (format.fmt.raw_data[17] & 1)) == 0)
	    {
		    ret = -1;
		    return ret; 
	    }
	    if ((format.fmt.raw_data[16]&(1<<4)) != 0) {
		    *system = TVD_PAL;
	    } else if ((format.fmt.raw_data[16] & (1<<4)) == 0) {
		    *system = TVD_NTSC;  
	    }
	    break;
    }
    return ret;
}

int TVDecoderService::v4l2SetVideoParams()
{
    int ret = UNKNOWN_ERROR;
    struct v4l2_format v_format;
    memset(&v_format, 0, sizeof(v_format));
    if(mPreviewHardware == 0)
    {
        mFormat = TVD_PL_YUV420;
    }
    if(mMirror)
    {
        mFormat = TVD_PL_YUV420;
    }
    v_format.type = V4L2_BUF_TYPE_PRIVATE;
    v_format.fmt.raw_data[0] = mInterface;
    v_format.fmt.raw_data[1] = mSystem;
    v_format.fmt.raw_data[2] = mFormat;

    switch(mChannel)
    {
        case TVD_CHANNEL_ONLY_1:
            v_format.fmt.raw_data[8]  = 1;	        //row
            v_format.fmt.raw_data[9]  = 1;	        //column
            v_format.fmt.raw_data[10] = 1;		//channel_index0
            v_format.fmt.raw_data[11] = 0;		//channel_index1
            v_format.fmt.raw_data[12] = 0;		//channel_index2
            v_format.fmt.raw_data[13] = 0;		//channel_index3
            break;
        case TVD_CHANNEL_ONLY_2:
            v_format.fmt.raw_data[8]  = 1;	        //row
            v_format.fmt.raw_data[9]  = 1;	        //column
            v_format.fmt.raw_data[10] = 0;		//channel_index0
            v_format.fmt.raw_data[11] = 1;		//channel_index1
            v_format.fmt.raw_data[12] = 0;		//channel_index2
            v_format.fmt.raw_data[13] = 0;		//channel_index3
            break;
        case TVD_CHANNEL_ONLY_3:
            v_format.fmt.raw_data[8]  = 1;	        //row
            v_format.fmt.raw_data[9]  = 1;	        //column
            v_format.fmt.raw_data[10] = 0;          //channel_index0
            v_format.fmt.raw_data[11] = 0;          //channel_index1
            v_format.fmt.raw_data[12] = 1;          //channel_index2
            v_format.fmt.raw_data[13] = 0;          //channel_index3
            break;
        case TVD_CHANNEL_ONLY_4:
            v_format.fmt.raw_data[8]  = 1;	        //row
            v_format.fmt.raw_data[9]  = 1;	        //column
            v_format.fmt.raw_data[10] = 0;          //channel_index0
            v_format.fmt.raw_data[11] = 0;          //channel_index1
            v_format.fmt.raw_data[12] = 0;          //channel_index2
            v_format.fmt.raw_data[13] = 1;          //channel_index3
            break;
        case TVD_CHANNEL_ALL_2x2:
            v_format.fmt.raw_data[8]  = 2;	        //row
            v_format.fmt.raw_data[9]  = 2;	        //column
            v_format.fmt.raw_data[10] = 1;          //channel_index0
            v_format.fmt.raw_data[11] = 2;          //channel_index1
            v_format.fmt.raw_data[12] = 3;          //channel_index2
            v_format.fmt.raw_data[13] = 4;          //channel_index3
            break;
        case TVD_CHANNEL_ALL_1x4:
            v_format.fmt.raw_data[8]  = 1;	        //row
            v_format.fmt.raw_data[9]  = 4;	        //column
            v_format.fmt.raw_data[10] = 1;          //channel_index0
            v_format.fmt.raw_data[11] = 2;          //channel_index1
            v_format.fmt.raw_data[12] = 3;          //channel_index2
            v_format.fmt.raw_data[13] = 4;          //channel_index3
            break;
        case TVD_CHANNEL_ALL_4x1:
            v_format.fmt.raw_data[8]  = 4;	        //row
            v_format.fmt.raw_data[9]  = 1;	        //column
            v_format.fmt.raw_data[10] = 1;          //channel_index0
            v_format.fmt.raw_data[11] = 2;          //channel_index1
            v_format.fmt.raw_data[12] = 3;          //channel_index2
            v_format.fmt.raw_data[13] = 4;          //channel_index 3
            break;
	case TVD_CHANNEL_ALL_1x2:
	    v_format.fmt.raw_data[8]  = 1;	        //row
	    v_format.fmt.raw_data[9]  = 2;	        //column
	    v_format.fmt.raw_data[10] = 1;          //channel_index0
	    v_format.fmt.raw_data[11] = 2;          //channel_index1
	    v_format.fmt.raw_data[12] = 0;          //channel_index2
	    v_format.fmt.raw_data[13] = 0;          //channel_index 3
	    break;
	case TVD_CHANNEL_ALL_2x1:
	    v_format.fmt.raw_data[8]  = 2;	        //row
	    v_format.fmt.raw_data[9]  = 1;	        //column
	    v_format.fmt.raw_data[10] = 1;          //channel_index0
	    v_format.fmt.raw_data[11] = 2;          //channel_index1
	    v_format.fmt.raw_data[12] = 0;          //channel_index2
	    v_format.fmt.raw_data[13] = 0;          //channel_index 3
	    break;

	default:
            break;
    }
    if (-1 == ioctl (mV4l2Handle, VIDIOC_S_FMT, &v_format))
    {
        ALOGE("VIDIOC_S_FMT error!\n");
        ret = -1;
        return ret;
    }

    ALOGV("#########################interface %d  system %d format %d\n",interface,system,format);
    struct v4l2_streamparm parms;
    v4l2SetColor();
    if(mFormat==TVD_MB_YUV420)
    {
        parms.type = V4L2_BUF_TYPE_PRIVATE;
        parms.parm.raw_data[0] = TVD_UV_SWAP;
        parms.parm.raw_data[1] = 0;
        if(ioctl(mV4l2Handle, VIDIOC_S_PARM, &parms)==-1)
        {
            ALOGE("VIDIOC_S_PARM error!\n");
            ret = -1;
            return ret;
        }
    }
    else
    {
        parms.type = V4L2_BUF_TYPE_PRIVATE;
        parms.parm.raw_data[0] = TVD_UV_SWAP;
        parms.parm.raw_data[1] = 1;
        if(ioctl(mV4l2Handle, VIDIOC_S_PARM, &parms)==-1)
        {
            ALOGE("VIDIOC_S_PARM error!\n");
            ret = -1;
            return ret;
        }
    }
    return OK;
}

int TVDecoderService::v4l2ReqBufs()
{
    int ret = UNKNOWN_ERROR;
    struct v4l2_requestbuffers rb;
    ALOGV("TO VIDIOC_REQBUFS count: %d", mBufferCnt);
    memset(&rb, 0, sizeof(rb));
    rb.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rb.memory = V4L2_MEMORY_MMAP;
    rb.count  = mBufferCnt;

    ret = ioctl(mV4l2Handle, VIDIOC_REQBUFS, &rb);
    if (ret < 0)
    {
        ALOGE("Init: VIDIOC_REQBUFS failed: %s", strerror(errno));
        return ret;
    }
    if (mBufferCnt != rb.count)
    {
        mBufferCnt = rb.count;
        ALOGD("VIDIOC_REQBUFS count: %d", mBufferCnt);
    }
    return OK;
}

int TVDecoderService::v4l2QueryBuf()
{
    int ret = UNKNOWN_ERROR;
    struct v4l2_buffer buf;
    
    for (unsigned int i = 0; i < mBufferCnt; i++)
    {
        memset (&buf, 0, sizeof (struct v4l2_buffer));
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = i;
        
        ret = ioctl (mV4l2Handle, VIDIOC_QUERYBUF, &buf);
        if (ret < 0)
        {
            ALOGE("Unable to query buffer (%s)", strerror(errno));
            return ret;
        }

        mMapMem.mem[i] = mmap (0,buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,mV4l2Handle,buf.m.offset);
        mMapMem.length = buf.length;
        ALOGV("index: %d, mem: %x, len: %x, offset: %x", i, (int)mMapMem.mem[i], buf.length, buf.m.offset);

        if (mMapMem.mem[i] == MAP_FAILED)
        {
            ALOGE("Unable to map buffer (%s)", strerror(errno));
            return -1;
        }
    
        ret = ioctl(mV4l2Handle, VIDIOC_QBUF, &buf);
        if (ret < 0)
        {
            ALOGE("VIDIOC_QBUF Failed");
            return ret;
        }
        getGeometry();
        if(mMirror)
        {
            int buffer_len = mVideoWidth * mVideoHeight * 3 / 2;
            mVideoBuffer.buf_vir_addr[i] = (int)cedara_phymalloc_map(buffer_len, 1024);
            mVideoBuffer.buf_phy_addr[i] = cedarv_address_vir2phy((void*)mVideoBuffer.buf_vir_addr[i]);
            mVideoBuffer.buf_phy_addr[i] |= 0x40000000;
            ALOGV("video buffer: index: %d, vir: %x, phy: %x, len: %x",i, mVideoBuffer.buf_vir_addr[i], mVideoBuffer.buf_phy_addr[i], buffer_len);
            memset((void*)mVideoBuffer.buf_vir_addr[i], 0x10, mVideoWidth * mVideoHeight);
            memset((void*)(mVideoBuffer.buf_vir_addr[i] + mVideoWidth * mVideoHeight),0x80, mVideoWidth * mVideoHeight / 2);
        }
    }
    return OK;
}

int TVDecoderService::v4l2StartStreaming()
{
    int ret = UNKNOWN_ERROR;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

      ret = ioctl(mV4l2Handle, VIDIOC_STREAMON, &type);
    if (ret < 0)
    {
        ALOGE("StartStreaming: Unable to start capture: %s",strerror(errno));
        return ret;
    }
    return OK;
}

int TVDecoderService::v4l2StopStreaming()
{
    int ret = UNKNOWN_ERROR;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = ioctl (mV4l2Handle, VIDIOC_STREAMOFF, &type);
    if (ret < 0)
    {
        ALOGE("StopStreaming: Unable to stop capture: %s", strerror(errno));
        return ret;
    }
    ALOGV("V4L2Camera::v4l2StopStreaming OK");
    return OK;
}

int TVDecoderService::v4l2UnmapBuf()
{
    int ret = UNKNOWN_ERROR;

    for (unsigned int i = 0; i < mBufferCnt; i++)
    {
        ret = munmap(mMapMem.mem[i], mMapMem.length);
                if (ret < 0)
        {
            ALOGE("v4l2CloseBuf Unmap failed");
            return ret;
        }
    }
    return OK;
}

int TVDecoderService::v4l2SetColor()
{
    struct v4l2_streamparm parms;
    int ret = UNKNOWN_ERROR;
    int tmp_hue=0;
    if(mHue>=50&&mHue<=100)
        tmp_hue=mHue-50;
    else if(mHue>=0&&mHue<50)
        tmp_hue=50-mHue;
    parms.type = V4L2_BUF_TYPE_PRIVATE;
    parms.parm.raw_data[0] = TVD_COLOR_SET;
    parms.parm.raw_data[1] = mLuma/1.2;			        //luma(0~255)
    parms.parm.raw_data[2] = mContrast*255/100;			//contrast(0~255)
    parms.parm.raw_data[3] = mSaturation*255/100;			//saturation(0~255)
    parms.parm.raw_data[4] = tmp_hue*256/100;		        //hue(0~255)
    if(ioctl(mV4l2Handle, VIDIOC_S_PARM, &parms)==-1)
    {
        ALOGE("VIDIOC_S_PARM error! line %d\n",__LINE__);
        ret = -1;
        return ret;
    }
    return OK;
}

void TVDecoderService::releasePreviewFrame(int index)
{
    struct v4l2_buffer buf;
    int ret = UNKNOWN_ERROR;
    memset(&buf, 0, sizeof(v4l2_buffer));
    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = index;
    ret = ioctl(mV4l2Handle, VIDIOC_QBUF, &buf);
    if (ret != 0)
    {
        ALOGE("releasePreviewFrame: VIDIOC_QBUF Failed: index = %d, ret = %d, %s",buf.index, ret, strerror(errno));
    }
}

int TVDecoderService::v4l2WaitCamerReady()
{
    fd_set fds;
    struct timeval tv;
    int r;

    FD_ZERO(&fds);
    FD_SET(mV4l2Handle, &fds);

    /* Timeout */
    tv.tv_sec  = 2;
    tv.tv_usec = 0;

    r = select(mV4l2Handle + 1, &fds, NULL, NULL, &tv);
    if (r == -1)
    {
        ALOGE("select err");
        return -1;
    }
    else if (r == 0)
    {
        ALOGE("select timeout");
        return -1;
    }
    return 0;
}

int TVDecoderService::getPreviewFrame(v4l2_buffer *buf)
{
    int ret = UNKNOWN_ERROR;
    
    buf->type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf->memory = V4L2_MEMORY_MMAP;

    ret = ioctl(mV4l2Handle, VIDIOC_DQBUF, buf);
    if (ret < 0)
    {
        ALOGE("GetPreviewFrame: VIDIOC_DQBUF Failed");
        return __LINE__; 			                // can not return false
    }

    if(mMirror)
    {
        g2d_blt		blit_para;
        int 		err;
        
        blit_para.src_image.addr[0]      = (unsigned int)buf->m.offset;
        blit_para.src_image.addr[1]      = (unsigned int)buf->m.offset + mVideoWidth * mVideoHeight;
        blit_para.src_image.w            = mVideoWidth;	                /* src buffer width in pixel units */
        blit_para.src_image.h            = mVideoHeight;	        /* src buffer height in pixel units */
        blit_para.src_image.format       = G2D_FMT_PYUV420UVC;
        blit_para.src_image.pixel_seq    = G2D_SEQ_NORMAL;              /* not use now */
        blit_para.src_rect.x             = 0;				/* src rect->x in pixel */
        blit_para.src_rect.y             = 0;				/* src rect->y in pixel */
        blit_para.src_rect.w             = mVideoWidth;			/* src rect->w in pixel */
        blit_para.src_rect.h             = mVideoHeight;		/* src rect->h in pixel */
    
        blit_para.dst_image.addr[0]      = mVideoBuffer.buf_phy_addr[buf->index];
        blit_para.dst_image.addr[1]      = mVideoBuffer.buf_phy_addr[buf->index] + mVideoWidth * mVideoHeight;
        blit_para.dst_image.w            = mVideoWidth;	                /* dst buffer width in pixel units */
        blit_para.dst_image.h            = mVideoHeight;	        /* dst buffer height in pixel units */
        blit_para.dst_image.format       = G2D_FMT_PYUV420UVC;
        blit_para.dst_image.pixel_seq    = G2D_SEQ_NORMAL;              /* not use now */
    //	blit_para.dst_image.pixel_seq    = (mVideoFormat == V4L2_PIX_FMT_NV12) ? G2D_SEQ_NORMAL : G2D_SEQ_VUVU;          /* not use now */
        blit_para.dst_x                  = 0;				/* dst rect->x in pixel */
        blit_para.dst_y                  = 0;				/* dst rect->y in pixel */
        blit_para.color                  = 0xff;          		/* fix me*/
        blit_para.alpha                  = 0xff;                        /* globe alpha */
    
        blit_para.flag = G2D_BLT_FLIP_HORIZONTAL; // G2D_BLT_FLIP_HORIZONTAL;
    
        err = ioctl(mG2DHandle,G2D_CMD_BITBLT,(unsigned long)&blit_para);
        if(err < 0)
        {		
            ALOGE("ioctl, G2D_CMD_BITBLT failed");
            //return -1;
        }
    }
    return OK;
}

status_t TVDecoderService::setParameter(tvd_interface_t interface,tvd_system_t system,tvd_fmt_t format,tvd_channel_t channel,int mirror)
{
    mInterface      = interface;
    mSystem         = system;
    mFormat         = format;
    mChannel        = channel;
    mMirror         = mirror;
    return OK;
}

int TVDecoderService::getGeometry(void)
{
    struct v4l2_format format;
    int ret = UNKNOWN_ERROR;
    
    memset(&format, 0, sizeof(format));
    format.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(mV4l2Handle, VIDIOC_G_FMT, &format);
    if (ret < 0)
    {
        ALOGE("VIDIOC_G_FMT Failed: %s", strerror(errno));
        return ret;
    }

    mVideoWidth     = format.fmt.pix.width;
    mVideoHeight    = format.fmt.pix.height;
    ALOGV("Set buffer geometry # mVideoWidth: %d, mVideoHeight: %d\n",mVideoWidth,mVideoHeight);
    return OK;
}

status_t TVDecoderService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) {
    return BnTVDecoderService::onTransact(code, data, reply, flags);
}

}; // namespace android
