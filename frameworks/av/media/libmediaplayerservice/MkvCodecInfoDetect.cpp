//#define ALOG_NDEBUG 0
#define ALOG_TAG "MkvCodecInfoDetect"

#include "MatroskaExtractor.h"
#include "string.h"
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#include <utils/String8.h>

namespace android {

bool MkvGetCodecId(int fd, int64_t offset, int64_t length)
{
	status_t ret = false;
	sp<DataSource> dataSource = new FileSource(dup(fd), offset, length);
    status_t err = dataSource->initCheck();
    if (err != OK) {
        return err;
    }

    AString sniffedMIME;
    sniffedMIME.setTo(MEDIA_MIMETYPE_CONTAINER_MATROSKA);
    sp<MediaExtractor> extractor = MediaExtractor::Create(
            dataSource, sniffedMIME.empty() ? NULL : sniffedMIME.c_str());

    if (extractor == NULL) {
        return UNKNOWN_ERROR;
    }

    for (size_t i = 0; i < extractor->countTracks(); ++i) {
    	sp<MetaData> meta = extractor->getTrackMetaData(i);
        const char *_mime;
        meta->findCString(kKeyMIMEType, &_mime);

        String8 mime = String8(_mime);
        if (!strncasecmp(mime.string(), "video/", 6)) {
            if (!strcasecmp(mime.string(), MEDIA_MIMETYPE_VIDEO_VP9)) {
            	ALOGD("kKeyMIMEType %s", mime.string());
            	ret = true;
            }
        }
    }

    if (dataSource != NULL) {
    	dataSource.clear();
    	dataSource = NULL;
    }
    if (extractor != NULL) {
    	extractor.clear();
    	extractor = NULL;
    }
    return ret;
}





}
