/*
	 droid vnc server - Android VNC server
	 Copyright (C) 2009 Jose Pereira <onaips@gmail.com>

	 This library is free software; you can redistribute it and/or
	 modify it under the terms of the GNU Lesser General Public
	 License as published by the Free Software Foundation; either
	 version 3 of the License, or (at your option) any later version.

	 This library is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 Lesser General Public License for more details.

	 You should have received a copy of the GNU Lesser General Public
	 License along with this library; if not, write to the Free Software
	 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
	 */

#include "flinger.h"
#include "screenFormat.h"

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <binder/IMemory.h>
#if 0
#include <surfaceflinger/ISurfaceComposer.h>
#include <surfaceflinger/SurfaceComposerClient.h>
#else
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#endif

#if 0
struct Info {
    size_t      size;
    size_t      bitsPerPixel;
    struct {
        uint8_t     ah;
        uint8_t     al;
        uint8_t     rh;
        uint8_t     rl;
        uint8_t     gh;
        uint8_t     gl;
        uint8_t     bh;
        uint8_t     bl;
    };
    uint8_t     components;
};

static Info const sPixelFormatInfos[] = {
        { 0,  0, { 0, 0,   0, 0,   0, 0,   0, 0 }, 0 },
        { 4, 32, {32,24,   8, 0,  16, 8,  24,16 }, PixelFormatInfo::RGBA },
        { 4, 24, { 0, 0,   8, 0,  16, 8,  24,16 }, PixelFormatInfo::RGB  },
        { 3, 24, { 0, 0,   8, 0,  16, 8,  24,16 }, PixelFormatInfo::RGB  },
        { 2, 16, { 0, 0,  16,11,  11, 5,   5, 0 }, PixelFormatInfo::RGB  },
        { 4, 32, {32,24,  24,16,  16, 8,   8, 0 }, PixelFormatInfo::RGBA },
        { 2, 16, { 1, 0,  16,11,  11, 6,   6, 1 }, PixelFormatInfo::RGBA },
        { 2, 16, { 4, 0,  16,12,  12, 8,   8, 4 }, PixelFormatInfo::RGBA },
        { 1,  8, { 8, 0,   0, 0,   0, 0,   0, 0 }, PixelFormatInfo::ALPHA},
        { 1,  8, { 0, 0,   8, 0,   8, 0,   8, 0 }, PixelFormatInfo::L    },
        { 2, 16, {16, 8,   8, 0,   8, 0,   8, 0 }, PixelFormatInfo::LA   },
        { 1,  8, { 0, 0,   8, 5,   5, 2,   2, 0 }, PixelFormatInfo::RGB  },
};

    // bpp, red, green, blue, alpha
    static const int format_map[][9] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0},   // INVALID
        {32, 0, 8, 8, 8, 16, 8, 24, 8}, // HAL_PIXEL_FORMAT_RGBA_8888
        {32, 0, 8, 8, 8, 16, 8, 0, 0}, // HAL_PIXEL_FORMAT_RGBX_8888
        {24, 16, 8, 8, 8, 0, 8, 0, 0},  // HAL_PIXEL_FORMAT_RGB_888
        {16, 11, 5, 5, 6, 0, 5, 0, 0},  // HAL_PIXEL_FORMAT_RGB_565
        {32, 16, 8, 8, 8, 0, 8, 24, 8}, // HAL_PIXEL_FORMAT_BGRA_8888
        {16, 11, 5, 6, 5, 1, 5, 0, 1},  // HAL_PIXEL_FORMAT_RGBA_5551
        {16, 12, 4, 8, 4, 4, 4, 0, 4}   // HAL_PIXEL_FORMAT_RGBA_4444
    };
#endif

using namespace android;

ScreenshotClient *screenshotClient=NULL;

extern "C" screenFormat getscreenformat_flinger()
{
  //get format on PixelFormat struct
	PixelFormat f=screenshotClient->getFormat();

#if 0
	PixelFormatInfo pf;
	getPixelFormatInfo(f,&pf);
#endif

	screenFormat format;

#if 0
	format.bitsPerPixel = pf.bitsPerPixel;
	format.width = screenshotClient->getWidth();
	format.height =     screenshotClient->getHeight();
	format.size = pf.bitsPerPixel*format.width*format.height/CHAR_BIT;
	format.redShift = pf.l_red;
	format.redMax = pf.h_red;
	format.greenShift = pf.l_green;
	format.greenMax = pf.h_green-pf.h_red;
	format.blueShift = pf.l_blue;
	format.blueMax = pf.h_blue-pf.h_green;
	format.alphaShift = pf.l_alpha;
	format.alphaMax = pf.h_alpha-pf.h_blue;
#else
	format.bitsPerPixel = bitsPerPixel(f);
	format.width = screenshotClient->getWidth();
	format.height =     screenshotClient->getHeight();
	format.size = bitsPerPixel(f)*format.width*format.height/CHAR_BIT;
	switch (f) {
	case PIXEL_FORMAT_RGBA_8888:
		/* {ah, al, rh, rl, gh, gl, bh, bl} */
		/* {32,24,   8, 0,  16, 8,  24,16 } */
		format.redShift = 0;
		format.redMax = 8;
		format.greenShift = 8;
		format.greenMax = 8;
		format.blueShift = 16;
		format.blueMax = 8;
		format.alphaShift = 24;
		format.alphaMax = 8;
		break;
	case PIXEL_FORMAT_RGBX_8888:
		/* {ah, al, rh, rl, gh, gl, bh, bl} */
		/* { 0, 0,   8, 0,  16, 8,  24,16 } */
		format.redShift = 0;
		format.redMax = 8;
		format.greenShift = 8;
		format.greenMax = 8;
		format.blueShift = 16;
		format.blueMax = 8;
		format.alphaShift = 0;
		format.alphaMax = 0;
		break;
	case PIXEL_FORMAT_BGRA_8888:
		/* {ah, al, rh, rl, gh, gl, bh, bl} */
		/* {32,24,  24,16,  16, 8,   8, 0 } */
		format.redShift = 16;
		format.redMax = 8;
		format.greenShift = 8;
		format.greenMax = 8;
		format.blueShift = 0;
		format.blueMax = 8;
		format.alphaShift = 24;
		format.alphaMax = 8;
		break;
	case PIXEL_FORMAT_RGB_888:
		/* {ah, al, rh, rl, gh, gl, bh, bl} */
		/* { 0, 0,   8, 0,  16, 8,  24,16 } */
		format.redShift = 16;
		format.redMax = 8;
		format.greenShift = 8;
		format.greenMax = 8;
		format.blueShift = 0;
		format.blueMax = 8;
		format.alphaShift = 0;
		format.alphaMax = 0;
		break;
	case PIXEL_FORMAT_RGB_565:
		/* {ah, al, rh, rl, gh, gl, bh, bl} */
		/* { 0, 0,  16,11,  11, 5,   5, 0 } */
		format.redShift = 11;
		format.redMax = 5;
		format.greenShift = 5;
		format.greenMax = 6;
		format.blueShift = 0;
		format.blueMax = 5;
		format.alphaShift = 0;
		format.alphaMax = 0;
		break;
	case PIXEL_FORMAT_RGBA_5551:
		/* {ah, al, rh, rl, gh, gl, bh, bl} */
		/* { 1, 0,  16,11,  11, 6,   6, 1 } */
		format.redShift = 11;
		format.redMax = 5;
		format.greenShift = 6;
		format.greenMax = 5;
		format.blueShift = 1;
		format.blueMax = 5;
		format.alphaShift = 0;
		format.alphaMax = 1;
		break;
	case PIXEL_FORMAT_RGBA_4444:
		/* {ah, al, rh, rl, gh, gl, bh, bl} */
		/* { 4, 0,  16,12,  12, 8,   8, 4 } */
		format.redShift = 12;
		format.redMax = 4;
		format.greenShift = 8;
		format.greenMax = 4;
		format.blueShift = 4;
		format.blueMax = 4;
		format.alphaShift = 0;
		format.alphaMax = 4;
		break;
	}
#endif

	return format;
}


extern "C" int init_flinger()
{
	int errno;

	const String16 name("SurfaceFlinger");
	sp<ISurfaceComposer> composer;
	getService(name, &composer);
	sp<IBinder> display(composer->getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain));

	L("--Initializing gingerbread access method--\n");

  screenshotClient = new ScreenshotClient();
	errno = screenshotClient->update(display);
  if (!screenshotClient->getPixels())
    return -1;

  if (errno != NO_ERROR) {
		return -1;
	}
	return 0;
}

extern "C" unsigned int *readfb_flinger()
{
	const String16 name("SurfaceFlinger");
	sp<ISurfaceComposer> composer;
	getService(name, &composer);
	sp<IBinder> display(composer->getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain));

	screenshotClient->update(display);
	return (unsigned int*)screenshotClient->getPixels();
}

extern "C" void close_flinger()
{
  free(screenshotClient);
}
