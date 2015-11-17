/*******************************************************************************
--                                                                            --
--                    CedarX Multimedia Framework                             --
--                                                                            --
--          the Multimedia Framework for Linux/Android System                 --
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                         Softwinner Products.                               --
--                                                                            --
--                   (C) COPYRIGHT 2011 SOFTWINNER PRODUCTS                   --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
*******************************************************************************/

#ifndef VIDEO_RENDER_H_
#define VIDEO_RENDER_H_

enum PARAMETER_KEY{
	VIDEO_RENDER_UNKOWN_PARA  = 0,
	//get frame id, return frame id.
	VIDEO_RENDER_GET_FRAME_ID = 1,
	//get output type from GUI,0 is returned
	//if using GPU, 1 is returned if using DE.
	VIDEO_RENDER_GET_OUTPUT_TYPE = 2,
	//set video buffers, so that gui can output
	//buffers from decoder to DE directly.
	VIDEO_RENDER_SET_VIDEO_BUFFERS_INFO = 3,
	//set gui buffer dimensions.
	VIDEO_RENDER_SET_VIDEO_BUFFERS_DIMENSIONS = 4
};

typedef struct CDX_VideoRenderHAL{
	const char *info;
	void *callback_info;
	int  vr4sft_first_frame;

	int (*init)(struct CDX_VideoRenderHAL *handle, int width, int height, int format);
	void (*exit)(struct CDX_VideoRenderHAL *handle);
	int (*render)(struct CDX_VideoRenderHAL *handle, void *frame_info, int frame_id);
	int (*perform)(struct CDX_VideoRenderHAL *handle, int para_id, void *data);

    int (*dequeue_frame)(struct CDX_VideoRenderHAL *handle, void *frame_info, void *pANativeWindowBuffer);    //request frame from gpu, ANativeWindowBufferCedarXWrapper
    int (*enqueue_frame)(struct CDX_VideoRenderHAL *handle, void *pANativeWindowBuffer);    //release frame to gpu, ANativeWindowBufferCedarXWrapper
}CDX_VideoRenderHAL;

CDX_VideoRenderHAL *cedarx_video_render_create(void *callback_info);
void cedarx_video_render_destroy(void *handle);

#endif
