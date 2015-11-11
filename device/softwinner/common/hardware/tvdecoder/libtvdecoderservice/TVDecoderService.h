#ifndef ANDROID_HARDWARE_TVDECODERSERVICE_H
#define ANDROID_HARDWARE_TVDECODERSERVICE_H

#include <linux/videodev.h> 
#include <string.h>
#include <utils/RefBase.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurface.h>
#include <gui/Surface.h>
#include <gui/SurfaceTextureClient.h>
#include <ui/FramebufferNativeWindow.h>
#include <binder/MemoryHeapBase.h>
#include <binder/ProcessState.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/BinderService.h>

#include "../include/ITVDecoderService.h"

namespace android {
class TVDecoderService;
typedef enum
{
    TVD_UV_SWAP             = 0,
    TVD_COLOR_SET           = 1,
}tvd_param_t;

typedef enum
{
    TVD_CVBS                = 0,
    TVD_YPBPR_I             = 1,
    TVD_YPBPR_P             = 2,
}tvd_interface_t;

typedef enum
{
    TVD_NTSC                = 0,
    TVD_PAL                 = 1,
    TVD_SECAM               = 2,
}tvd_system_t;        
typedef enum
{
    TVD_PL_YUV420           = 0,
    TVD_MB_YUV420           = 1,
    TVD_PL_YUV422           = 2,
}tvd_fmt_t;
typedef enum
{
    TVD_CHANNEL_ONLY_1      = 0,
    TVD_CHANNEL_ONLY_2      = 1,
    TVD_CHANNEL_ONLY_3      = 2, 
    TVD_CHANNEL_ONLY_4      = 3,
    TVD_CHANNEL_ALL_2x2     = 4,
    TVD_CHANNEL_ALL_1x4     = 5,
    TVD_CHANNEL_ALL_4x1     = 6,
    TVD_CHANNEL_ALL_1x2	    = 7,	
    TVD_CHANNEL_ALL_2x1	    = 8,
}tvd_channel_t;

class TVDecoderService :
    public BinderService<TVDecoderService>,
    public BnTVDecoderService
{
    friend class                    BinderService<TVDecoderService>;
public:
    static char const*              getServiceName() {return "media.tvd";}
                                    TVDecoderService();
    virtual                         ~TVDecoderService();

    virtual status_t                connect(int pid,int previewhw,int intf,int fmt,int chan);
    virtual status_t                disconnect(int pid);
    virtual status_t                startDecoder();
    virtual status_t                stopDecoder();
    virtual status_t                setSize(int x,int y,int w,int h);
    virtual status_t                setColor(int luma,int contrast,int saturation,int hue);
    virtual status_t                setPreviewDisplay(const sp<Surface>& surface);
    virtual status_t                setParameter(tvd_interface_t interface,tvd_system_t system,tvd_fmt_t format,tvd_channel_t channel,int mirror);

    virtual status_t                onTransact(uint32_t code, const Parcel& data,Parcel* reply, uint32_t flags);
    virtual void                    onFirstRef();

    class TVDecoderPreviewThread : public Thread 
    {
        public:
            TVDecoderPreviewThread(TVDecoderService* ts) : Thread(false),mService(ts) 
            {
            }	  
            virtual void onFirstRef() 
            {
                run("TVDecoderPreviewThread", PRIORITY_URGENT_DISPLAY);
            }
            virtual bool threadLoop() 
            {
                mService->tvDecoderPreviewThread();
                return true;
            }
        private:
            TVDecoderService* mService;
    };
private:
    void                            initDefaultParameters(void);
    int                             openCameraDev();
    void                            closeCameraDev();
    int                             getSystem(int *system);
    int                             v4l2SetVideoParams();
    int                             v4l2ReqBufs();
    int                             v4l2QueryBuf();
    int                             v4l2StartStreaming();
    int                             v4l2StopStreaming();
    int                             v4l2UnmapBuf();
    int                             v4l2SetColor();
    int                             getFrameRate();
    void                            releasePreviewFrame(int index);
    int                             tryFmtSize(int * width, int * height);
    int                             v4l2WaitCamerReady();
    int                             getPreviewFrame(v4l2_buffer *buf);
    status_t                        tvDecoderPreviewThread();
    status_t                        setLayerAddr(v4l2_buffer * buf);
    int                             getGeometry(void);
    sp<ANativeWindow>               getNativeWindow();
    status_t                        resetTVDecoder();
private:
    Mutex                           mLock;
    int                             mPreviewHardware;
    bool                            mFirstFrame;
    int                             mScreenX;
    int                             mScreenY;
    int                             mScreenWidth;
    int                             mScreenHeight;

    int                             mVideoWidth;
    int                             mVideoHeight;
    int                             mVideoFormat;

    int                             mV4l2Handle;	
    unsigned int                    mBufferCnt;
    int                             mOpened;
    sp<TVDecoderPreviewThread>      mTVDecoderPreviewThread;

    int                             mPid;
    int                             mConnectFinished;
    int                             mConnecting;
    int                             mFirstTime;
    bool                            mTVDecoderPreviewThreadStarted;
    sp<ANativeWindow>               mPreviewWindow;
    int                             mG2DHandle;        
    int                             mMirror;        
    int                             mInterface;
    int                             mSystem;
    int                             mFormat;
    int                             mChannel;     
    int                             mBuffes_Geometry_Format;
    int                             mLuma;
    int                             mContrast;
    int                             mSaturation;
    int                             mHue;
    typedef struct bufferManagerQ_t
    {
        int     buf_vir_addr[5];
        int     buf_phy_addr[5];
        int     write_id;
        int     read_id;
        int     buf_unused;
    }bufferManagerQ_t;
    bufferManagerQ_t                mVideoBuffer;
        
    typedef struct v4l2_mem_map_t{
        void *	mem[5]; 
        int 	length;
    }v4l2_mem_map_t;
    v4l2_mem_map_t                  mMapMem;

    sp<Surface>                     mSurface;
    sp<SurfaceComposerClient>       mComposerClient;
    sp<SurfaceControl>              mControl;
};
}  // namespace android


extern "C" int cedarx_hardware_init(int mode);
extern "C" int cedarx_hardware_exit(int mode);
extern "C" void *cedara_phymalloc_map(unsigned int size, int align);
extern "C" void cedara_phyfree_map(void *buf);
extern "C" unsigned int cedarv_address_vir2phy(void *addr);
extern "C" unsigned int cedarv_address_phy2vir(void *addr);


#endif // ANDROID_HARDWARE_TVDECODERSERVICE_H

