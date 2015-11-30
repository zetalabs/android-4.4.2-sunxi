$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

DEVICE_PACKAGE_OVERLAYS := \
    device/softwinner/wing-common/overlay

include device/softwinner/wing-common/prebuild/tools/tools.mk

PRODUCT_PACKAGES += \
    libion \
    hwcomposer.wing \
    lights.wing \
    sensors.wing 

# file system
PRODUCT_PACKAGES += \
    setup_fs \
	e2fsck \
	libext2fs \
	libext2_blkid \
	libext2_uuid \
	libext2_profile \
	libext2_com_err \
	libext2_e2p \
	make_ext4fs

PRODUCT_PACKAGES += \
    libwvm \
    com.android.future.usb.accessory

# audio
PRODUCT_PACKAGES += \
    audio.primary.wing \
    audio.a2dp.default \
	audio.usb.default  \
	audio.r_submix.default 

PRODUCT_PACKAGES += \
	libsrec_jni

PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/hardware/audio/audio_policy.conf:system/etc/audio_policy.conf \
	device/softwinner/wing-common/overlay/frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml

# To support the installation of Google_Googles.apk
PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/prebuild/google/system/framework/com.google.android.maps.jar:system/framework/com.google.android.maps.jar \
	device/softwinner/wing-common/prebuild/google/system/etc/permissions/com.google.android.maps.xml:system/framework/com.google.android.maps.xml

# camera
PRODUCT_PACKAGES += \
    camera.wing \

# video
PRODUCT_PACKAGES += \
	libaudioutils \
	libcedarxbase \
	libcedarxosal \
	libcedarv \
	libcedarv_base \
	libcedarv_adapter \
	libve \
	libaw_audio \
	libaw_audioa \
	libswdrm \
	libstagefright_soft_cedar_h264dec \
	libfacedetection \
	libthirdpartstream \
	libcedarxsftstream \
    libion_alloc \
	libjpgenc \
	libaw_h264enc \
	libI420colorconvert \
	libstagefrighthw \
	libOmxCore \
	libOmxVenc \
	libOmxVdec \

#FaceLock
PRODUCT_PACKAGES += \
    FaceLock

# miracast
PRODUCT_COPY_FILES += \
    device/softwinner/wing-common/wfd_blacklist.conf:system/etc/wfd_blacklist.conf

# System app

# common apk
PRODUCT_PACKAGES += \
    Update \
    FileExplore

# baidu video
PRODUCT_PACKAGES += \
	libAudiofe_V1.so \
	libcyberplayer.so \
	libffmpeg.so \
	libgetcpuspec.so \
	libp2p-jni.so \
	libstlport_shared.so

# preinstall apk
PRODUCT_PACKAGES += \
    flashplayer.apk \
	DragonFire.apk \
	DragonPhone.apk \
    Chrome.apk

# egl
PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/egl/gralloc.sun7i.so:system/lib/hw/gralloc.sun7i.so \
	device/softwinner/wing-common/egl/libMali.so:system/lib/libMali.so \
	device/softwinner/wing-common/egl/libUMP.so:system/lib/libUMP.so \
	device/softwinner/wing-common/egl/egl.cfg:system/lib/egl/egl.cfg \
	device/softwinner/wing-common/egl/libEGL_mali.so:system/lib/egl/libEGL_mali.so \
	device/softwinner/wing-common/egl/libGLESv1_CM_mali.so:system/lib/egl/libGLESv1_CM_mali.so \
	device/softwinner/wing-common/egl/libGLESv2_mali.so:system/lib/egl/libGLESv2_mali.so \

PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/media_codecs.xml:system/etc/media_codecs.xml \
	device/softwinner/wing-common/gps.conf:system/etc/gps.conf \
	device/softwinner/wing-common/chrome-command-line:/system/etc/chrome-command-line

PRODUCT_COPY_FILES += \
    device/softwinner/wing-common/sensors.sh:system/bin/sensors.sh

# init.rc, kernel
PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/init.rc:root/init.rc \
    device/softwinner/wing-common/init.sun7i.usb.rc:root/init.sun7i.usb.rc

PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/init_parttion.sh:root/sbin/init_parttion.sh \
	
PRODUCT_COPY_FILES += \
		frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \

PRODUCT_PROPERTY_OVERRIDES += \
	persist.sys.strictmode.visual=0 \
	persist.sys.strictmode.disable=1

PRODUCT_PROPERTY_OVERRIDES += \
	ro.kernel.android.checkjni=0

PRODUCT_PROPERTY_OVERRIDES += \
	wifi.interface=wlan0 \
	wifi.supplicant_scan_interval=15 \
	keyguard.no_require_sim=true

PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=131072

PRODUCT_PROPERTY_OVERRIDES += \
	ro.crypto.sw2hwkeymaster=true

# Enabling type-precise GC results in larger optimized DEX files.  The
# additional storage requirements for ".odex" files can cause /system
# to overflow on some devices, so this is configured separately for
# each product.
PRODUCT_TAGS += dalvik.gc.type-precise

# if DISPLAY_BUILD_NUMBER := true then
# BUILD_DISPLAY_ID := $(BUILD_ID).$(BUILD_NUMBER)
# required by gms.
DISPLAY_BUILD_NUMBER := true
BUILD_NUMBER := $(shell date +%Y%m%d)

PRODUCT_PACKAGES += \
		PartnerBookmarksProvider

PRODUCT_PROPERTY_OVERRIDES += \
drm.service.enabled=true

PRODUCT_PACKAGES += \
com.google.widevine.software.drm.xml \
com.google.widevine.software.drm \
libdrmwvmplugin \
libwvm \
libWVStreamControlAPI_L3 \
libwvdrm_L3 \
libdrmdecrypt \
libwvdrmengine