# full wing product config
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)
$(call inherit-product, device/softwinner/common/sw-common.mk)

DEVICE_PACKAGE_OVERLAYS := \
    device/softwinner/wing-common/overlay

include device/softwinner/wing-common/prebuild/tools/tools.mk

# ext4 filesystem utils
PRODUCT_PACKAGES += \
	e2fsck \
	libext2fs \
	libext2_blkid \
	libext2_uuid \
	libext2_profile \
	libext2_com_err \
	libext2_e2p \
	make_ext4fs

PRODUCT_PACKAGES += \
	audio.primary.exDroid \
	audio.a2dp.default \
	audio.usb.default \
	audio.r_submix.default

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
	libsunxi_alloc \
	libsrec_jni \
	libjpgenc \
	libstagefrighthw \
	libOmxCore \
	libOmxVdec \
	libOmxVenc \
	libaw_h264enc \
	libI420colorconvert \
	Camera \
	libjni_mosaic \
	FileExplore \
	u3gmonitor \
	rild \
	chat \
	libjni_pinyinime \
	libcnr

PRODUCT_PACKAGES += \
	libjni_hmm_shared_engine \
	libjni_googlepinyinime_latinime_5 \
	libjni_googlepinyinime_5 \
	libjni_delight \
	libgnustl_shared

PRODUCT_PACKAGES += \
	LiveWallpapersPicker \
	LiveWallpapers \
	android.software.live_wallpaper.xml \
	FileExplore

# softwinner	
PRODUCT_PACKAGES +=  \
   android.softwinner.framework.jar \
   SoftWinnerService.apk \
   backup/SoftWinnerService.apk \
   libsoftwinner_servers.so \
   libupdatesoftwinner.so \
   librtsp.so \
   libjni_fireair.so \
   updatesoftwinner 

PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/media_codecs.xml:system/etc/media_codecs.xml \
	device/softwinner/common/hardware/audio/audio_policy.conf:system/etc/audio_policy.conf \
	device/softwinner/common/hardware/audio/phone_volume.conf:system/etc/phone_volume.conf

#exdroid HAL
PRODUCT_PACKAGES += \
   libion \
   hwcomposer.exDroid \
   display.exdroid \
   camera.exDroid \
   sensors.exDroid \
   lights.sun7i \
   display.sun7i

#common System APK
PRODUCT_PACKAGES += \
   FileExplore \
   4KPlayer \
   AWGallery \
   AWUpdate \
   GooglePinyin \
   YGGameCenter \
   YGServiceFramework

#install apk's lib to system/lib
PRODUCT_PACKAGES +=  \
   libjni_eglfence_awgallery.so \
   libjni_mosaic.so

#preinstall APK
PRODUCT_PACKAGES += \
   DragonPhone.apk \
   DragonFire.apk \
   VideoTest.apk

PRODUCT_COPY_FILES += \
	device/softwinner/common/hardware/audio/audio_policy.conf:system/etc/audio_policy.conf \
	device/softwinner/common/hardware/audio/phone_volume.conf:system/etc/phone_volume.conf

# keylayout
PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/axp20-supplyer.kl:system/usr/keylayout/axp20-supplyer.kl

#egl
PRODUCT_COPY_FILES += \
        device/softwinner/wing-common/egl/libMali.so:system/lib/libMali.so \
        device/softwinner/wing-common/egl/libUMP.so:system/lib/libUMP.so \
        device/softwinner/wing-common/egl/libEGL_mali.so:system/lib/egl/libEGL_mali.so \
        device/softwinner/wing-common/egl/libGLESv1_CM_mali.so:system/lib/egl/libGLESv1_CM_mali.so \
        device/softwinner/wing-common/egl/libGLESv2_mali.so:system/lib/egl/libGLESv2_mali.so \
        device/softwinner/wing-common/egl/gralloc.sun7i.so:system/lib/hw/gralloc.sun7i.so \
        device/softwinner/wing-common/egl/egl.cfg:system/lib/egl/egl.cfg \

# for boot nand/card auto detect 
PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/busybox:root/sbin/busybox \
	device/softwinner/wing-common/init_parttion.sh:root/sbin/init_parttion.sh \
	device/softwinner/wing-common/busybox:recovery/root/sbin/busybox \
	device/softwinner/wing-common/init_parttion.sh:recovery/root/sbin/init_parttion.sh \
	

#premission feature
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.location.xml:system/etc/permissions/android.hardware.location.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.xml:system/etc/permissions/android.hardware.touchscreen.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
		frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \

PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/preinstall.sh:/system/bin/preinstall.sh \
	device/softwinner/wing-common/sensors.sh:/system/bin/sensors.sh \
	device/softwinner/wing-common/data_resume.sh:/system/bin/data_resume.sh \
	device/softwinner/wing-common/chrome-command-line:/system/etc/chrome-command-line
	
# wfd no invite
PRODUCT_COPY_FILES += \
    device/softwinner/wing-common/wfd_blacklist.conf:system/etc/wfd_blacklist.conf

# init.rc
PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/init.rc:root/init.rc

# kernel
PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/init.sun7i.usb.rc:root/init.sun7i.usb.rc

# table core hardware
PRODUCT_COPY_FILES += \
    device/softwinner/wing-common/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml

PRODUCT_PROPERTY_OVERRIDES += \
	persist.sys.strictmode.visual=0 \
	persist.sys.strictmode.disable=1

PRODUCT_PROPERTY_OVERRIDES += \
	ro.opengles.version=131072

PRODUCT_PROPERTY_OVERRIDES += \
	ro.kernel.android.checkjni=0

PRODUCT_PROPERTY_OVERRIDES += \
   debug.force.software.rending=true

# Enabling type-precise GC results in larger optimized DEX files.  The
# additional storage requirements for ".odex" files can cause /system
# to overflow on some devices, so this is configured separately for
# each product.
PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_PROPERTY_OVERRIDES += \
	ro.reversion.aw_sdk_tag=exdroid4.4_r1.2-a20-v4.5 \
	ro.sys.cputype=DualCore-A20Series

PRODUCT_PROPERTY_OVERRIDES += \
	ro.crypto.sw2hwkeymaster=true \
	ro.build.selinux=true \
	wifi.interface=wlan0 \
	wifi.supplicant_scan_interval=15 \
	keyguard.no_require_sim=true

PRODUCT_PROPERTY_OVERRIDES += \
	persist.demo.hdmirotationlock=0

PRODUCT_PROPERTY_OVERRIDES += \
	rild.libargs=-d/dev/ttyUSB2 \
	rild.libpath=/system/lib/libsoftwinner-ril.so \

# if DISPLAY_BUILD_NUMBER := true then
# BUILD_DISPLAY_ID := $(BUILD_ID).$(BUILD_NUMBER)
# required by gms.
DISPLAY_BUILD_NUMBER := true
BUILD_NUMBER := $(shell date +%Y%m%d)

PRODUCT_PACKAGES += \
    PartnerBookmarksProvider

# for drm
PRODUCT_PROPERTY_OVERRIDES += \
    drm.service.enabled=true

PRODUCT_PACKAGES += \
    com.google.widevine.software.drm.xml \
    com.google.widevine.software.drm \
    libdrmwvmplugin \
    libwvdrmengine \
    libwvm \
    libWVStreamControlAPI_L3 \
    libwvdrm_L3 \
    libdrmdecrypt
