#define LOG_TAG "Tvd"
#include <android/window.h>
#include <jni.h>
#include <JNIHelp.h>
#include <utils/Log.h>
#include <utils/threads.h>
#include <gui/Surface.h>
#include <fcntl.h> 
#include <sys/mman.h> 
#include <system/window.h>
#include <utils/misc.h>

#include "../include/TVDecoderClient.h"

#ifndef NELEM
# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

namespace android {
TVDecoderClient * mClient = new TVDecoderClient();

static jint softwinner_tv_TVDecoder_connect(JNIEnv * env,jobject jobj,jint previewhw,jint intf,jint fmt,jint chan)
{
    if(mClient == NULL)
    {
        ALOGE("TVDecoderClient is NULL");
        return -1;
    }
    return mClient->connect(previewhw,intf,fmt,chan);
}

static jint softwinner_tv_TVDecoder_disconnect(JNIEnv * env,jobject jobj)
{
    if(mClient == NULL)
    {
        ALOGE("TVDecoderClient is NULL");
        return -1;
    }
    return mClient->disconnect();
}

static jint softwinner_tv_TVDecoder_startDecoder(JNIEnv * env,jobject jobj)
{
    if (mClient == NULL)
    {
        ALOGE("TVDecoderClient is NULL");
        return -1;
    }
    return mClient->startDecoder();
}

static jint softwinner_tv_TVDecoder_stopDecoder(JNIEnv * env,jobject jobj)
{
    if (mClient == NULL)
    {
        ALOGE("TVDecoderClient is NULL");
        return -1;
    }
    return mClient->stopDecoder();
}

static jint softwinner_tv_TVDecoder_setColor(JNIEnv * env,jobject jobj,jint luma,jint contrast,jint saturation,jint hue)
{
    if (mClient == NULL)
    {
        ALOGE("TVDecoderClient is NULL");
        return -1;
    }
    return mClient->setColor(luma,contrast,saturation,hue);
}

static jint softwinner_tv_TVDecoder_setSize(JNIEnv * env,jobject jobj,jint x,jint y,jint w,jint h)
{
    if (mClient == NULL)
    {
        ALOGE("TVDecoderClient is NULL");
        return -1;
    }
    return mClient->setSize(x,y,w,h);
}

static jint softwinner_tv_TVDecoder_setPreviewDisplay(JNIEnv * env,jobject jobj,jobject jSurface)
{
    jclass clazz = env->FindClass("android/view/Surface");
    jfieldID field_surface;
    field_surface = env->GetFieldID(clazz, ANDROID_VIEW_SURFACE_JNI_ID, "I");
    if(field_surface==NULL)
    {
        ALOGE("########################field_surface is null\n");
        return -1;
    }
    sp<IGraphicBufferProducer> bufferProducer = NULL;
    if (jSurface != NULL)
    {
#if 0
        sp<Surface> surface(android_view_Surface_getSurface(env, jSurface));
        if (surface != NULL) {
            new_st = surface->getIGraphicBufferProducer();
            if (new_st == NULL) {
                jniThrowException(env, "java/lang/IllegalArgumentException",
                    "The surface does not have a binding SurfaceTexture!");
                return -1;
            }
            new_st->incStrong((void*)decVideoSurfaceRef);
        } else {
            jniThrowException(env, "java/lang/IllegalArgumentException",
                    "The surface has been released");
            return -1;
        }
#else
        sp<Surface> surface = reinterpret_cast<Surface*>(env->GetIntField(jSurface, field_surface));
        bufferProducer = surface->getIGraphicBufferProducer();
#endif
    }
    if (mClient->setPreviewDisplay(bufferProducer) != NO_ERROR)
    {
        //jniThrowException(env, "java/io/IOException", "setPreviewDisplay failed");
    }
    return 0;
}

static JNINativeMethod sMethods[] = {
    {"connect",           "(IIII)I",                   (void*)softwinner_tv_TVDecoder_connect},
    {"disconnect",        "()I",                       (void*)softwinner_tv_TVDecoder_disconnect},
    {"startDecoder",      "()I",                       (void*)softwinner_tv_TVDecoder_startDecoder},
    {"stopDecoder",       "()I",                       (void*)softwinner_tv_TVDecoder_stopDecoder},
    {"setColor",          "(IIII)I",                   (void*)softwinner_tv_TVDecoder_setColor},
    {"setSize",           "(IIII)I",                   (void*)softwinner_tv_TVDecoder_setSize},
    {"setPreviewDisplay", "(Landroid/view/Surface;)I", (void*)softwinner_tv_TVDecoder_setPreviewDisplay},
};

int register_softwinner_tv_TVDecoder(JNIEnv* env)
{
    return jniRegisterNativeMethods(env, "softwinner/tv/TVDecoder",sMethods, NELEM(sMethods));
}

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("GetEnv failed!");
        return result;
    }
    ALOG_ASSERT(env, "Could not retrieve the env!");
    register_softwinner_tv_TVDecoder(env);
    return JNI_VERSION_1_4;
}
}
