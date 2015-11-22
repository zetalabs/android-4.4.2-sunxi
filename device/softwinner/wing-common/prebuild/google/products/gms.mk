# Add Google Apps and Services
PRODUCT_PACKAGES += \
	CalendarGoogle \
	GoogleContactsSyncAdapter \
	GoogleEars \
	LatinImeGoogle \
	MediaUploader \
	Music2 \
    Chrome \
    BrowserProviderProxy \
	FaceLock

#not use
#GalleryGoogle
#	Books \
#	Magazines \
#	PlayGames \
#	PlusOne \
#	Videos \
#	YouTube \
#	Gmail2 \
#	GenieWidget \
#	Wallet \
#	Drive \
#	GoogleHome \
#	Keep \
#	QuickOffice \
#	GoogleEarth \
#	Hangouts \
#	Maps \
#	Street \


#priv apps	
PRODUCT_PACKAGES += \
	GoogleCalendarProvider \
	GoogleBackupTransport \
	GoogleFeedback \
	GoogleLoginService \
	GooglePartnerSetup \
	GoogleServicesFramework \
	GoogleOneTimeInitializer \
	Phonesky \
	PrebuiltGmsCore \
	talkback \
	Velvet \
	SetupWizard


#frameworks jar
PRODUCT_PACKAGES += \
	com.google.android.media.effects.jar \
	google_generic_update.txt

#google libs
PRODUCT_PACKAGES += \
	libstlport_shared \
	librs.antblur_constant \
	libdocsimageutils \
	libgoogle_recognizer_jni_l \
	libwebp_android \
	libfilterpack_facedetect \
	libpatts_engine_jni_api \
	libmoviemaker-jni \
	libspeexwrapper.so \
	libndk1 \
	libfrsdk \
	librs.antblur_drama \
	libplus_jni_v8 \
	librs.drama \
	libnetjni \
	libAppDataSearch \
	liblinearalloc \
	libearthmobile \
	libfilterframework_jni \
	libjni_t13n_shared_engine \
	libwebrtc_audio_coding \
	librs.image_wrapper \
	librs.retrolux \
	libvorbisencoder \
	librs.fixedframe \
    libchromeview \
	libwebrtc_audio_preprocessing \
	libgames_rtmp_jni \
	librsjni \
	librs.antblur \
	librs.film_base \
	libfacetracker \
	libspeexwrapper \
	libearthandroid \
	librectifier-v7a \
	libjni_latinimegoogle \
	librs.grey \
	libWVphoneAPI \
	libocrclient \
	libvcdecoder_jni \
	libdocscanner_image-v7a \
	libvideochat_jni \
	libRSSupport \
	libfacelock_jni.so
	
#not use
#libjni_eglfence
#libjni_filtershow_filters

#usr data
PRODUCT_PACKAGES += \
	phonelist \
	compile_grammar.config \
	chrome-command-line \
	clg \
	grammar.config \
	dictation.config \
	hotword.config \
	normalizer \
	hotword_classifier \
	hotword_normalizer \
	hmmlist \
	commands.abnf \
	hclg_shotword \
	hotword_prompt.txt \
	endpointer_voicesearch.config \
	contacts.abnf \
	c_fst \
	hotword_word_symbols \
	phone_state_map \
	rescoring_lm \
	endpointer_dictation.config \
	offensive_word_normalizer \
	hmm_symbols \
	ep_acoustic_model \
	wordlist \
	metadata \
	dict \
	norm_fst \
	dnn \
	g2p_fst
 	


# Include GoogleTTS and TTS languages needed for GoogleTTS
PRODUCT_PACKAGES += GoogleTTS
include device/softwinner/wing-common/prebuild/google/products/text_to_speech_languages.mk

# Overlay for Google network and fused location providers
$(call inherit-product, device/sample/products/location_overlay.mk)

# Overrides
PRODUCT_PROPERTY_OVERRIDES += \
	ro.setupwizard.mode=OPTIONAL \
	ro.com.google.gmsversion=4.4
