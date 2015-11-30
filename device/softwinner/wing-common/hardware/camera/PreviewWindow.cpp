
#include "CameraDebug.h"
#if DBG_PREVIEW
#define LOG_NDEBUG 0
#endif
#define LOG_TAG "PreviewWindow"
#include <cutils/log.h>

#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>

#include "V4L2CameraDevice.h"
#include "PreviewWindow.h"


namespace android {

/*#define CAMHAL_GRALLOC_USAGE GRALLOC_USAGE_HW_TEXTURE | \
								 GRALLOC_USAGE_HW_RENDER | \
								 GRALLOC_USAGE_SW_READ_RARELY | \
								 GRALLOC_USAGE_SW_WRITE_NEVER
*/
#define CAMHAL_GRALLOC_USAGE GRALLOC_USAGE_SW_READ_RARELY | \
									 GRALLOC_USAGE_SW_WRITE_NEVER

#if SCALE_PREVIEW_ENABLE
static void scale(const void *src, void *dst, unsigned int width, unsigned int height, int scalermode)
{
	// param  scalermode: 2, 4;
	int i,j;
	unsigned char *psrc = (unsigned char *)src;
	unsigned char *pdst = (unsigned char *)dst;  
	uint16_t * psrc_uv;
	uint16_t * pdst_uv;
	int neon_height_y;
	int neon_height_uv;
	int neon_width;
	int neon_jump_width_y;
	int neon_jump_width_uv;
	unsigned char tmp[32];
	int scale = 8;

	if(scalermode == 2)
	{
		if(width%16 != 0)
		{
			LOGE("NOt support width");
			return;
		}
		
		neon_height_y = (int)height/2;
		neon_height_uv = (int)height/4;
		neon_width = (int)width/16;
		neon_jump_width_y = width;
		neon_jump_width_uv = width>>1;
		// y
		for(i = 0; i < neon_height_y; i++)
		{
			for(j=0; j<neon_width;j++)
			{
				asm volatile (

					"vld2.8         {d0 - d1}, [%[psrc]]         \n\t"
					"vst1.8         d0, [%[pdst]]                \n\t"
					: [psrc] "+r" (psrc), [pdst] "+r" (pdst)
					:  //[srcY] "r" (srcY)
					: "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
				);

				pdst += 8;
				psrc += 16;
			}
			psrc += neon_jump_width_y;
		}
		//uv
		psrc_uv = (uint16_t *)psrc;
		pdst_uv = (uint16_t *)pdst;
		
		//psrc_uv = (uint16_t *)(src+1280*720);
		//pdst_uv = (uint16_t *)(dst+1280*720/4);

		for(i = 0; i < neon_height_uv; i++)
		{
			for(j=0; j< neon_width;j++)
			{
				asm volatile (
				
					"vld2.u16		   {d0 - d1}, [%[psrc_uv]]		   \n\t"
					"vdup.16		   d10, %[scale]				   \n\t"
					"vshl.u16		   d11, d0, d10 				   \n\t"
					//"vshr.u16 	   d12, d0, d10 				   \n\t"
					"vsri.16		   d11,  d0,	#8				   \n\t"		
					"vst1.u8		   d11, [%[pdst_uv]]			   \n\t"
					//"vshl.i16 	   d4, d0,	  #8				   \n\t"
					//"vsri.16		   d4, d0,	  #8				   \n\t"
					//"vst1.u16 	   d4, [%[pdst_uv]] 			   \n\t"
				
					: [psrc_uv] "+r" (psrc_uv), [pdst_uv] "+r" (pdst_uv), [scale] "+r" (scale)
					:  //[srcY] "r" (srcY)
					: "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
				);

				
				pdst_uv += 4;
				psrc_uv += 8;	
			}
			psrc_uv += neon_jump_width_uv;
		}		
	}
	else
	{
		if(width%32 != 0)
		{
			LOGE("NOt support width");
			return;
		}
		
		neon_height_y = (int)height/4;
		neon_height_uv = (int)height/8;
		neon_width = (int)width/32;
		neon_jump_width_y = width*3;
		neon_jump_width_uv = width*3/2;

		// y
		for(i = 0; i < neon_height_y; i++)
		{
			for(j=0; j<neon_width;j++)
			{
				asm volatile (

					"vld4.8         {d0 - d3}, [%[psrc]]         \n\t"
					"vst1.8         d0, [%[pdst]]              \n\t"
					: [psrc] "+r" (psrc), [pdst] "+r" (pdst)
					:  //[srcY] "r" (srcY)
					: "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
				);
					
				pdst += 8;
				psrc += 32;	
			}
			psrc += neon_jump_width_y;
		}

		//uv
		psrc_uv = (uint16_t *)psrc;
		pdst_uv = (uint16_t *)pdst;

		for(i = 0; i < neon_height_uv; i++)
		{
			for(j=0; j< neon_width;j++)
			{
				asm volatile (

					"vld4.u16          {d0 - d3}, [%[psrc_uv]]         \n\t"
					"vdup.16           d10, %[scale]                   \n\t"
					"vshl.u16		   d11, d0, d10					   \n\t"
					//"vshr.u16		   d12, d0, d10					   \n\t"
					"vsri.16           d11,  d0,    #8           	   \n\t"
							
					"vst1.u8           d11, [%[pdst_uv]]                \n\t"
					//"vshl.i16        d4, d0,    #8            	   \n\t"
					//"vsri.16         d4, d0,    #8           		   \n\t"
					//"vst1.u16        d4, [%[pdst_uv]]                \n\t"
			
					: [psrc_uv] "+r" (psrc_uv), [pdst_uv] "+r" (pdst_uv), [scale] "+r" (scale)
					:  //[srcY] "r" (srcY)
					: "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
				);

				pdst_uv += 4;
				psrc_uv += 16;	
			}
			psrc_uv += neon_jump_width_uv;
		}		
	}
}

#endif

static int calculateFrameSize(int width, int height, uint32_t pix_fmt)
{
	int frame_size = 0;
	switch (pix_fmt) {
		case V4L2_PIX_FMT_YVU420:
		case V4L2_PIX_FMT_YUV420:
		case V4L2_PIX_FMT_NV21:
		case V4L2_PIX_FMT_NV12:
			frame_size = (ALIGN_16B(width) * height * 12) / 8;
			break;
		case V4L2_PIX_FMT_YUYV:
			frame_size = (width * height) << 1;
			break;
		default:
			ALOGE("%s: Unknown pixel format %d(%.4s)",
				__FUNCTION__, pix_fmt, reinterpret_cast<const char*>(&pix_fmt));
			break;
	}
	return frame_size;
}

static void NV12ToNV21(const void* nv12, void* nv21, int width, int height)
{       
    char * src_uv = (char *)nv12 + width * height;
    char * dst_uv = (char *)nv21 + width * height;

    memcpy(nv21, nv12, width * height);

    for(int i = 0; i < width * height / 2; i += 2)
    {
           *(dst_uv + i) = *(src_uv + i + 1);
           *(dst_uv + i + 1) = *(src_uv + i);
    }
}
   
static void NV12ToNV21_shift(const void* nv12, void* nv21, int width, int height)
{       
    char * src_uv = (char *)nv12 + width * height;
    char * dst_uv = (char *)nv21 + width * height;

    memcpy(nv21, nv12, width * height);
    memcpy(dst_uv, src_uv + 1, width * height / 2 - 1);
}

static void formatToNV21(void *dst,
		               void *src,
		               int width,
		               int height,
		               size_t stride,
		               uint32_t offset,
		               unsigned int bytesPerPixel,
		               size_t length,
		               int pixelFormat)
{
    unsigned int alignedRow, row;
    unsigned char *bufferDst, *bufferSrc;
    unsigned char *bufferDstEnd, *bufferSrcEnd;
    uint16_t *bufferSrc_UV;

    unsigned int y_uv[2];
    y_uv[0] = (unsigned int)src;
	y_uv[1] = (unsigned int)src + width*height;

	// NV12 -> NV21
    if (pixelFormat == V4L2_PIX_FMT_NV12) {
        bytesPerPixel = 1;
        bufferDst = ( unsigned char * ) dst;
        bufferDstEnd = ( unsigned char * ) dst + width*height*bytesPerPixel;
        bufferSrc = ( unsigned char * ) y_uv[0] + offset;
        bufferSrcEnd = ( unsigned char * ) ( ( size_t ) y_uv[0] + length + offset);
        row = width*bytesPerPixel;
        alignedRow = stride-width;
        int stride_bytes = stride / 8;
        uint32_t xOff = offset % stride;
        uint32_t yOff = offset / stride;

        // going to convert from NV12 here and return
        // Step 1: Y plane: iterate through each row and copy
        for ( int i = 0 ; i < height ; i++) {
            memcpy(bufferDst, bufferSrc, row);
            bufferSrc += stride;
            bufferDst += row;
            if ( ( bufferSrc > bufferSrcEnd ) || ( bufferDst > bufferDstEnd ) ) {
                break;
            }
        }

        bufferSrc_UV = ( uint16_t * ) ((uint8_t*)y_uv[1] + (stride/2)*yOff + xOff);

        uint16_t *bufferDst_UV;

        // Step 2: UV plane: convert NV12 to NV21 by swapping U & V
        bufferDst_UV = (uint16_t *) (((uint8_t*)dst)+row*height);

        for (int i = 0 ; i < height/2 ; i++, bufferSrc_UV += alignedRow/2) {
            int n = width;
            asm volatile (
            "   pld [%[src], %[src_stride], lsl #2]                         \n\t"
            "   cmp %[n], #32                                               \n\t"
            "   blt 1f                                                      \n\t"
            "0: @ 32 byte swap                                              \n\t"
            "   sub %[n], %[n], #32                                         \n\t"
            "   vld2.8  {q0, q1} , [%[src]]!                                \n\t"
            "   vswp q0, q1                                                 \n\t"
            "   cmp %[n], #32                                               \n\t"
            "   vst2.8  {q0,q1},[%[dst]]!                                   \n\t"
            "   bge 0b                                                      \n\t"
            "1: @ Is there enough data?                                     \n\t"
            "   cmp %[n], #16                                               \n\t"
            "   blt 3f                                                      \n\t"
            "2: @ 16 byte swap                                              \n\t"
            "   sub %[n], %[n], #16                                         \n\t"
            "   vld2.8  {d0, d1} , [%[src]]!                                \n\t"
            "   vswp d0, d1                                                 \n\t"
            "   cmp %[n], #16                                               \n\t"
            "   vst2.8  {d0,d1},[%[dst]]!                                   \n\t"
            "   bge 2b                                                      \n\t"
            "3: @ Is there enough data?                                     \n\t"
            "   cmp %[n], #8                                                \n\t"
            "   blt 5f                                                      \n\t"
            "4: @ 8 byte swap                                               \n\t"
            "   sub %[n], %[n], #8                                          \n\t"
            "   vld2.8  {d0, d1} , [%[src]]!                                \n\t"
            "   vswp d0, d1                                                 \n\t"
            "   cmp %[n], #8                                                \n\t"
            "   vst2.8  {d0[0],d1[0]},[%[dst]]!                             \n\t"
            "   bge 4b                                                      \n\t"
            "5: @ end                                                       \n\t"
#ifdef NEEDS_ARM_ERRATA_754319_754320
            "   vmov s0,s0  @ add noop for errata item                      \n\t"
#endif
            : [dst] "+r" (bufferDst_UV), [src] "+r" (bufferSrc_UV), [n] "+r" (n)
            : [src_stride] "r" (stride_bytes)
            : "cc", "memory", "q0", "q1"
            );
        }
    }
}

DBG_TIME_AVG_BEGIN(TAG_CPY);
DBG_TIME_AVG_BEGIN(TAG_DQBUF);
DBG_TIME_AVG_BEGIN(TAG_LKBUF);
DBG_TIME_AVG_BEGIN(TAG_MAPPER);
DBG_TIME_AVG_BEGIN(TAG_EQBUF);
DBG_TIME_AVG_BEGIN(TAG_UNLKBUF);

PreviewWindow::PreviewWindow()
    : mPreviewWindow(NULL),
      mPreviewFrameWidth(0),
      mPreviewFrameHeight(0),
      mPreviewFrameSize(0),
      mCurPixelFormat(0),
      mPreviewEnabled(false),
      mShouldAdjustDimensions(true)
{
	F_LOG;
}

PreviewWindow::~PreviewWindow()
{
	F_LOG;
	mPreviewWindow = NULL;
}

/****************************************************************************
 * Camera API
 ***************************************************************************/

status_t PreviewWindow::setPreviewWindow(struct preview_stream_ops* window)
{
    LOGV("%s: current: %p -> new: %p", __FUNCTION__, mPreviewWindow, window);
	
    status_t res = NO_ERROR;
    Mutex::Autolock locker(&mObjectLock);

    /* Reset preview info. */
    mPreviewFrameWidth = mPreviewFrameHeight = 0;

    if (window != NULL) {
        /* The CPU will write each frame to the preview window buffer.
         * Note that we delay setting preview window buffer geometry until
         * frames start to come in. */
        res = window->set_usage(window, /*GRALLOC_USAGE_SW_WRITE_OFTEN*/CAMHAL_GRALLOC_USAGE);
        if (res != NO_ERROR) {
            window = NULL;
            res = -res; // set_usage returns a negative errno.
            LOGE("%s: Error setting preview window usage %d -> %s",
                 __FUNCTION__, res, strerror(res));
        }
    }
    mPreviewWindow = window;

    return res;
}

status_t PreviewWindow::startPreview()
{
    F_LOG;

    Mutex::Autolock locker(&mObjectLock);
    mPreviewEnabled = true;
	
	DBG_TIME_AVG_INIT(TAG_CPY);
	DBG_TIME_AVG_INIT(TAG_DQBUF);
	DBG_TIME_AVG_INIT(TAG_LKBUF);
	DBG_TIME_AVG_INIT(TAG_MAPPER);
	DBG_TIME_AVG_INIT(TAG_EQBUF);
	DBG_TIME_AVG_INIT(TAG_UNLKBUF);
    return NO_ERROR;
}

void PreviewWindow::stopPreview()
{
    F_LOG;

    Mutex::Autolock locker(&mObjectLock);
    mPreviewEnabled = false;
	mShouldAdjustDimensions = true;

	DBG_TIME_AVG_END(TAG_CPY, "copy ");
	DBG_TIME_AVG_END(TAG_DQBUF, "deque ");
	DBG_TIME_AVG_END(TAG_LKBUF, "lock ");
	DBG_TIME_AVG_END(TAG_MAPPER, "mapper ");
	DBG_TIME_AVG_END(TAG_EQBUF, "enque ");
	DBG_TIME_AVG_END(TAG_UNLKBUF, "unlock ");
}

/****************************************************************************
 * Public API
 ***************************************************************************/

bool PreviewWindow::onNextFrameAvailable(const void* frame)
{
    int res;
    Mutex::Autolock locker(&mObjectLock);

	V4L2BUF_t * pv4l2_buf = (V4L2BUF_t *)frame;

	int preview_format = 0;
	int preview_addr_phy = 0;
	int preview_addr_vir = 0;
	int preview_width = 0;
	int preview_height = 0;
	RECT_t preview_crop;

    if (!isPreviewEnabled() || mPreviewWindow == NULL) 
	{
        return true;
    }

	if ((pv4l2_buf->isThumbAvailable == 1)
		&& (pv4l2_buf->thumbUsedForPreview == 1))
	{
		preview_format = pv4l2_buf->thumbFormat;
		preview_addr_phy = pv4l2_buf->thumbAddrPhyY;
		preview_addr_vir = pv4l2_buf->thumbAddrVirY;
		preview_width = pv4l2_buf->thumbWidth;
		preview_height= pv4l2_buf->thumbHeight;
		memcpy((void*)&preview_crop, (void*)&pv4l2_buf->thumb_crop_rect, sizeof(RECT_t));
	}
	else
	{
		preview_format = pv4l2_buf->format;
		preview_addr_phy = pv4l2_buf->addrPhyY;
		preview_addr_vir = pv4l2_buf->addrVirY;
		preview_width = pv4l2_buf->width;
		preview_height= pv4l2_buf->height;
		memcpy((void*)&preview_crop, (void*)&pv4l2_buf->crop_rect, sizeof(RECT_t));
	}

    /* Make sure that preview window dimensions are OK with the camera device */
    if (adjustPreviewDimensions(pv4l2_buf) || mShouldAdjustDimensions) {
        LOGD("%s: Adjusting preview windows %p geometry to %dx%d",
             __FUNCTION__, mPreviewWindow, mPreviewFrameWidth,
             mPreviewFrameHeight);
		
		int format = V4L2_PIX_FMT_NV21;
		switch (preview_format)
		{
			case V4L2_PIX_FMT_NV21:
				LOGV("preview format: HAL_PIXEL_FORMAT_YCrCb_420_SP");
				format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
				break;
			case V4L2_PIX_FMT_NV12:
                #ifdef __SUN7I__     
                    LOGV("preview format: HAL_PIXEL_FORMAT_YCrCb_420_SP");
                    format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
				    break;
                #else
				    LOGV("preview format: V4L2_PIX_FMT_NV12");
				    format = 0x101;			// NV12
				    break;
                #endif
			case V4L2_PIX_FMT_YVU420:
			case V4L2_PIX_FMT_YUV420:	// to do
				LOGV("preview format: HAL_PIXEL_FORMAT_YV12");
				format = HAL_PIXEL_FORMAT_YV12;
				break;
			case V4L2_PIX_FMT_YUYV:
				LOGV("preview format: HAL_PIXEL_FORMAT_YCbCr_422_I");
				format = HAL_PIXEL_FORMAT_YCbCr_422_I;
				break;
			default:
				LOGE("preview unknown pixel format: %08x", preview_format);
				return false;
		}
	#if SCALE_PREVIEW_ENABLE
		int PreviewFrameWidth = mPreviewFrameWidth;
		int PreviewFrameHeight =mPreviewFrameHeight;
		if(mPreviewFrameWidth == 1920 && mPreviewFrameHeight ==1080)
		{
			PreviewFrameWidth=mPreviewFrameWidth /SCALE;
			PreviewFrameHeight=mPreviewFrameHeight /SCALE;
			LOGD("1080P,Scale the preview to a half: Adjusting preview windows geometry to %dx%d",
             		 PreviewFrameWidth, PreviewFrameHeight);
		}
        	res = mPreviewWindow->set_buffers_geometry(mPreviewWindow,
                                                   PreviewFrameWidth,
                                                   PreviewFrameHeight,
							format);			
	#else
        res = mPreviewWindow->set_buffers_geometry(mPreviewWindow,
                                                   mPreviewFrameWidth,
                                                   mPreviewFrameHeight,
												   format);
	#endif
        if (res != NO_ERROR) {
            LOGE("%s: Error in set_buffers_geometry %d -> %s",
                 __FUNCTION__, -res, strerror(-res));
            return false;
        }
		mShouldAdjustDimensions = false;

		res = mPreviewWindow->set_buffer_count(mPreviewWindow, 3);
		if (res != 0) 
		{
	        LOGE("native_window_set_buffer_count failed: %s (%d)", strerror(-res), -res);

	        if ( ENODEV == res ) {
	            LOGE("Preview surface abandoned!");
	            mPreviewWindow = NULL;
	        }

	        return false;
	    }
    }

    /*
     * Push new frame to the preview window.
     */
		
	DBG_TIME_AVG_AREA_IN(TAG_DQBUF);

    /* Dequeue preview window buffer for the frame. */
    buffer_handle_t* buffer = NULL;
    int stride = 0;
    res = mPreviewWindow->dequeue_buffer(mPreviewWindow, &buffer, &stride);
    if (res != NO_ERROR || buffer == NULL) {
        LOGE("%s: Unable to dequeue preview window buffer: %d -> %s",
            __FUNCTION__, -res, strerror(-res));

		int undequeued = 0;
		mPreviewWindow->get_min_undequeued_buffer_count(mPreviewWindow, &undequeued);
		LOGW("now undequeued: %d", undequeued);
		
        return false;
    }
	DBG_TIME_AVG_AREA_OUT(TAG_DQBUF);

	DBG_TIME_AVG_AREA_IN(TAG_LKBUF);

    /* Let the preview window to lock the buffer. */
    res = mPreviewWindow->lock_buffer(mPreviewWindow, buffer);
    if (res != NO_ERROR) {
        LOGE("%s: Unable to lock preview window buffer: %d -> %s",
             __FUNCTION__, -res, strerror(-res));
        mPreviewWindow->cancel_buffer(mPreviewWindow, buffer);
        return false;
    }
	DBG_TIME_AVG_AREA_OUT(TAG_LKBUF);
	
	DBG_TIME_AVG_AREA_IN(TAG_MAPPER);
	
    /* Now let the graphics framework to lock the buffer, and provide
     * us with the framebuffer data address. */
    void* img = NULL;
    const Rect rect(mPreviewFrameWidth, mPreviewFrameHeight);
    GraphicBufferMapper& grbuffer_mapper(GraphicBufferMapper::get());
    res = grbuffer_mapper.lock(*buffer, GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &img);
    if (res != NO_ERROR) {
        LOGE("%s: grbuffer_mapper.lock failure: %d -> %s",
             __FUNCTION__, res, strerror(res));
        mPreviewWindow->cancel_buffer(mPreviewWindow, buffer);
        return false;
    }

	DBG_TIME_AVG_AREA_OUT(TAG_MAPPER);
		
	mPreviewWindow->set_crop(mPreviewWindow, 
							preview_crop.left,
							preview_crop.top, 
							preview_crop.left + preview_crop.width,
							preview_crop.top + preview_crop.height);

	DBG_TIME_AVG_AREA_IN(TAG_CPY);

#ifdef USE_ION_MEM_ALLOCATOR
	ion_flush_cache((void*)preview_addr_vir, mPreviewFrameSize);
	// ion_flush_cache_all();
#elif USE_SUNXI_MEM_ALLOCATOR
	sunxi_flush_cache((void*)preview_addr_vir, mPreviewFrameSize);
	// sunxi_flush_cache_all();
#endif

	if (preview_format == V4L2_PIX_FMT_NV21
		|| preview_format == V4L2_PIX_FMT_NV12)
	{
		char * src = (char *)preview_addr_vir;
		char * dst = (char *)img;
    #ifdef __SUN7I__
        if (preview_format == V4L2_PIX_FMT_NV12)
    	{
            
        #if 1
		#if SCALE_PREVIEW_ENABLE
		// scale the 1080p preview window to a half of 1080p  and  NV12 <--> NV21
		if(preview_width == 1920 && preview_height == 1080)
		{
			scale((void *)preview_addr_vir,(void *)img,preview_width,preview_height,SCALE);
		}
		else
	    		formatToNV21((void *)img, 
						(void *)preview_addr_vir,
						preview_width, 
						preview_height,
						ALIGN_16B(preview_width),
						0,
						2,
						ALIGN_16B(preview_width) * preview_height * 3/2,
						preview_format);
		#else
    		// NV12 <--> NV21
    		formatToNV21((void *)img, 
    					(void *)preview_addr_vir,
    					preview_width, 
    					preview_height,
    					ALIGN_16B(preview_width),
    					0,
    					2,
    					ALIGN_16B(preview_width) * preview_height * 3/2,
    					preview_format);
		#endif
        #else
            NV12ToNV21_shift((void*)preview_addr_vir, (void*)img, preview_width, preview_height);
        #endif
        
    	}
        else
        {
            // y
    		memcpy(dst, src, ALIGN_16B(preview_width) * preview_height);

    		// uv
    		src = (char *)preview_addr_vir + ALIGN_16B(preview_width) * preview_height;
    		dst = (char *)img + (ALIGN_16B(preview_width)*preview_height);
    		memcpy(dst, src, ALIGN_16B(preview_width) * preview_height >> 1);
        }
    #else
            // y
		memcpy(dst, src, ALIGN_16B(preview_width) * preview_height);
		
		// uv
		src = (char *)preview_addr_vir + ALIGN_16B(preview_width) * preview_height;
		dst = (char *)img + ALIGN_4K(ALIGN_16B(preview_width)*preview_height);
		memcpy(dst, src, ALIGN_16B(preview_width) * preview_height >> 1);
            
    #endif
        
	}
	else if(preview_format == V4L2_PIX_FMT_YUV420)
	{
		// YU12 to YV12
		char * src = (char *)preview_addr_vir;
		char * dst = (char *)img;
		// y
		for (int h = 0; h < preview_height; h++)
		{
			memcpy(dst, src, preview_width);
			src += ALIGN_16B(preview_width);
			dst += ALIGN_32B(preview_width);
		}

		// v
		src = (char *)preview_addr_vir + ALIGN_16B(preview_width)*preview_height + ALIGN_16B(ALIGN_16B(preview_width)/2)*preview_height/2;// ALIGN_16B(preview_width)*preview_height*5/4;
		dst = (char *)img + ALIGN_32B(preview_width)*preview_height;
		memcpy(dst, src, ALIGN_16B(preview_width/2)*preview_height >> 1);
		
		// u
		src = (char *)preview_addr_vir + ALIGN_16B(preview_width)*preview_height;
		dst = (char *)img + ALIGN_32B(preview_width)*preview_height + (ALIGN_16B(preview_width/2)*preview_height >> 1);
		memcpy(dst, src, ALIGN_16B(preview_width/2)*preview_height >> 1);
	}
	else if(preview_format == V4L2_PIX_FMT_YVU420)
	{
		// YV12
		char * src = (char *)preview_addr_vir;
		char * dst = (char *)img;
		// y
		for (int h = 0; h < preview_height; h++)
		{
			memcpy(dst, src, preview_width);
			src += ALIGN_16B(preview_width);
			dst += ALIGN_32B(preview_width);
		}
		
		// u & v
		memcpy(dst, src, ALIGN_16B(preview_width/2)*preview_height);
	}
	else
	{
		LOGE("unknown preview format");
	}
	
	DBG_TIME_AVG_AREA_OUT(TAG_CPY);

	DBG_TIME_AVG_AREA_IN(TAG_EQBUF);

	mPreviewWindow->set_timestamp(mPreviewWindow, pv4l2_buf->timeStamp);
	mPreviewWindow->enqueue_buffer(mPreviewWindow, buffer);

	DBG_TIME_AVG_AREA_OUT(TAG_EQBUF);

	DBG_TIME_AVG_AREA_IN(TAG_UNLKBUF);

    grbuffer_mapper.unlock(*buffer);

	DBG_TIME_AVG_AREA_OUT(TAG_UNLKBUF);

	return true;
}

/***************************************************************************
 * Private API
 **************************************************************************/

bool PreviewWindow::adjustPreviewDimensions(V4L2BUF_t* pbuf)
{
	/* Match the cached frame dimensions against the actual ones. */
	if ((pbuf->isThumbAvailable == 1)
		&& (pbuf->thumbUsedForPreview == 1))		// use thumb frame for preview
	{
		if ((mPreviewFrameWidth == pbuf->thumbWidth)
			&& (mPreviewFrameHeight == pbuf->thumbHeight)
			&& (mCurPixelFormat == pbuf->thumbFormat)) 
		{
			/* They match. */
			return false;
		}

		LOGV("cru: [%d, %d], get: [%d, %d]", mPreviewFrameWidth, mPreviewFrameHeight,
			pbuf->thumbWidth, pbuf->thumbHeight);
		/* They don't match: adjust the cache. */
		mPreviewFrameWidth = pbuf->thumbWidth;
		mPreviewFrameHeight = pbuf->thumbHeight;
		mCurPixelFormat = pbuf->thumbFormat;

		mPreviewFrameSize = calculateFrameSize(pbuf->thumbWidth, pbuf->thumbHeight, pbuf->thumbFormat);
	}
	else
	{
	    if ((mPreviewFrameWidth == pbuf->width)
			&& (mPreviewFrameHeight == pbuf->height)
			&& (mCurPixelFormat == pbuf->format)) 
		{
	        /* They match. */
	        return false;
	    }

		LOGV("cru: [%d, %d], get: [%d, %d]", mPreviewFrameWidth, mPreviewFrameHeight,
			pbuf->width, pbuf->height);
	    /* They don't match: adjust the cache. */
	    mPreviewFrameWidth = pbuf->width;
	    mPreviewFrameHeight = pbuf->height;
		mCurPixelFormat = pbuf->format;

		mPreviewFrameSize = calculateFrameSize(pbuf->width, pbuf->height, pbuf->format);
	}

	mShouldAdjustDimensions = false;
    return true;
}

}; /* namespace android */
