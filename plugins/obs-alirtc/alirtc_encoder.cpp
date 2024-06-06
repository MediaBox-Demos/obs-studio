#include "alirtc_encoder.h"
#include "alirtc_common.h"
#include <util/dstr.h>


#define do_log(level, format, ...)                 \
	blog(level, "[qsv encoder: '%s'] " format, \
	     obs_encoder_get_name(obsalirtc->encoder), ##__VA_ARGS__)


struct obs_encoder_info openh264_encoder_info;
struct obs_encoder_info x264_encoder_info;
struct obs_encoder_info qsv_encoder_info;
struct obs_encoder_info nvidia_encoder_info;
struct obs_encoder_info x265_encoder_info;
struct obs_encoder_info s265_encoder_info;
struct obs_encoder_info opus_encoder_info;
struct obs_encoder_info aac_encoder_info;

struct obs_alirtc_video_encoder {
	obs_encoder_t *encoder;

	AlivcVideoCodecManufacturer codec_type;

	DARRAY(uint8_t) packet_data;

	uint8_t *extra_data;
	uint8_t *sei;

	size_t extra_data_size;
	size_t sei_size;

	// os_performance_token_t *performance_token;

	uint32_t roi_increment;
};

typedef struct obs_alirtc_audio_encoder {
	obs_encoder_t *encoder;

	int channels, sample_rate;


	uint64_t total_samples;

	int frame_size_bytes;

	uint8_t *packet_buffer;
	int packet_buffer_size;
} obs_alirtc_audio_encoder;


static const char *alirtc_openh264_getname(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return "AliRtc OpenH264";
}

static const char *alirtc_x264_getname(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return "AliRtc x264";
}

static const char *alirtc_qsv_getname(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return "AliRtc Qsv H264";
}

static const char *alirtc_nvidia_getname(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return "AliRtc Nvidia H264";
}

static const char *alirtc_x265_getname(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return "AliRtc x265";
}

static const char *alirtc_s265_getname(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return "AliRtc s265";
}

static void *obs_alirtc_encoder_create(AlivcVideoCodecManufacturer codec_type, obs_data_t *settings,
			    obs_encoder_t *encoder) {
	blog(LOG_INFO, "obs_alirtc_encoder_create codec_type %d ", codec_type);
    // AlivcLivePusher *pusher = obs_alirtc_get_instace();
    // if (pusher) {
    //     pusher->setVideoDecoderCodecManufacturerConfig(codec_type);
    // }

    struct obs_alirtc_video_encoder *video_encoder =
	    (struct obs_alirtc_video_encoder *)bzalloc(
		    sizeof(struct obs_alirtc_video_encoder));

    video_encoder->codec_type = codec_type;
    video_encoder->encoder = encoder;

    return video_encoder;
}

static void *alirtc_openh264_enc_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	return obs_alirtc_encoder_create(AlivcVideoCodecManufactureropenH264, settings, encoder);
}

static void *alirtc_x264_enc_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	return obs_alirtc_encoder_create(AlivcVideoCodecManufacturerX264, settings, encoder);
}

static void *alirtc_qsv_enc_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	return obs_alirtc_encoder_create(AlivcVideoCodecManufacturerQsvIntel, settings, encoder);
}

static void *alirtc_nvidia_enc_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	return obs_alirtc_encoder_create(AlivcVideoCodecManufacturerNvidia, settings, encoder);
}

static void *alirtc_x265_enc_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	return obs_alirtc_encoder_create(AlivcVideoCodecManufacturerX265, settings, encoder);
}

static void *alirtc_s265_enc_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	return obs_alirtc_encoder_create(AlivcVideoCodecManufacturerS265, settings, encoder);
}

static void *alirtc_opus_enc_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	blog(LOG_INFO, "alirtc_opus_enc_create encoder create");
	obs_alirtc_audio_encoder *audio_encoder =
		(obs_alirtc_audio_encoder *)bzalloc(
			sizeof(obs_alirtc_audio_encoder));

    audio_encoder->encoder = encoder;
    return audio_encoder;
}

static void *alirtc_aac_enc_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	blog(LOG_INFO, "alirtc_aac_enc_create encoder create");
	obs_alirtc_audio_encoder *audio_encoder =
		(obs_alirtc_audio_encoder *)bzalloc(
			sizeof(obs_alirtc_audio_encoder));

    audio_encoder->encoder = encoder;
    return audio_encoder;
}

static void alirtc_enc_destroy(void *data)
{
	bfree(data);

	blog(LOG_INFO, "alirtc_enc_destroy encoder destroyed");
}

static bool alirtc_enc_encode(void *data, struct encoder_frame *frame,
			  struct encoder_packet *packet, bool *received_packet)
{
	blog(LOG_INFO, "alirtc_enc_encode ");
    return true;
}

static size_t alirtc_enc_frame_size(void *data)
{
	blog(LOG_INFO, "alirtc_enc_frame_size ");
	return 0;
}

static void alirtc_enc_defaults(obs_data_t *settings)
{
	blog(LOG_INFO, "alirtc_enc_defaults ");
	// obs_data_set_default_int(settings, "bitrate", 128);
	// obs_data_set_default_bool(settings, "afterburner", true);
}

static bool alirtc_enc_extra_data(void *data, uint8_t **extra_data, size_t *size)
{
	blog(LOG_INFO, "alirtc_enc_extra_data ");
	// libfdk_encoder_t *enc = data;

	// *size = enc->info.confSize;
	// *extra_data = enc->info.confBuf;

	return true;
}

static void alirtc_enc_audio_info(void *data, struct audio_convert_info *info)
{
	blog(LOG_INFO, "alirtc_enc_audio_info ");
	UNUSED_PARAMETER(data);
	info->format = AUDIO_FORMAT_16BIT;
}


static void alirtc_video_info(void *data,
					struct video_scale_info *info)
{
	blog(LOG_INFO, "alirtc_video_info ");
	// struct obs_qsv *obsqsv = data;
	// enum video_format pref_format;

	// pref_format = obs_encoder_get_preferred_video_format(obsqsv->encoder);

	// if (!valid_av1_format(pref_format)) {
	// 	pref_format = valid_av1_format(info->format)
	// 			      ? info->format
	// 			      : VIDEO_FORMAT_NV12;
	// }

	// info->format = pref_format;
	// cap_resolution(obsqsv, info);
}

static bool alirtc_openh264_enc_update(void *data, obs_data_t *settings)
{
	blog(LOG_INFO, "alirtc_openh264_enc_update ");
	// struct obs_qsv *obsqsv = data;
	// obsqsv->params.nTargetBitRate =
	// 	(mfxU16)obs_data_get_int(settings, "bitrate");

	// if (!qsv_encoder_reconfig(obsqsv->context, &obsqsv->params)) {
	// 	warn("Failed to reconfigure");
	// 	return false;
	// }

	return true;
}

static bool alirtc_x264_enc_update(void *data, obs_data_t *settings)
{
	blog(LOG_INFO, "alirtc_x264_enc_update ");
	// struct obs_qsv *obsqsv = data;
	// obsqsv->params.nTargetBitRate =
	// 	(mfxU16)obs_data_get_int(settings, "bitrate");

	// if (!qsv_encoder_reconfig(obsqsv->context, &obsqsv->params)) {
	// 	warn("Failed to reconfigure");
	// 	return false;
	// }

	return true;
}

static bool alirtc_qsv_enc_update(void *data, obs_data_t *settings)
{
	blog(LOG_INFO, "alirtc_qsv_enc_update ");
	// struct obs_qsv *obsqsv = data;
	// obsqsv->params.nTargetBitRate =
	// 	(mfxU16)obs_data_get_int(settings, "bitrate");

	// if (!qsv_encoder_reconfig(obsqsv->context, &obsqsv->params)) {
	// 	warn("Failed to reconfigure");
	// 	return false;
	// }

	return true;
}

static bool alirtc_nvidia_enc_update(void *data, obs_data_t *settings)
{
	blog(LOG_INFO, "alirtc_nvidia_enc_update ");
	// struct obs_qsv *obsqsv = data;
	// obsqsv->params.nTargetBitRate =
	// 	(mfxU16)obs_data_get_int(settings, "bitrate");

	// if (!qsv_encoder_reconfig(obsqsv->context, &obsqsv->params)) {
	// 	warn("Failed to reconfigure");
	// 	return false;
	// }

	return true;
}

static bool alirtc_x265_enc_update(void *data, obs_data_t *settings)
{
	blog(LOG_INFO, "alirtc_x265_enc_update ");
	// struct obs_qsv *obsqsv = data;
	// obsqsv->params.nTargetBitRate =
	// 	(mfxU16)obs_data_get_int(settings, "bitrate");

	// if (!qsv_encoder_reconfig(obsqsv->context, &obsqsv->params)) {
	// 	warn("Failed to reconfigure");
	// 	return false;
	// }

	return true;
}

static bool alirtc_s265_enc_update(void *data, obs_data_t *settings)
{
	blog(LOG_INFO, "alirtc_s265_enc_update ");
	// struct obs_qsv *obsqsv = data;
	// obsqsv->params.nTargetBitRate =
	// 	(mfxU16)obs_data_get_int(settings, "bitrate");

	// if (!qsv_encoder_reconfig(obsqsv->context, &obsqsv->params)) {
	// 	warn("Failed to reconfigure");
	// 	return false;
	// }

	return true;
}


static bool alirtc_enc_sei(void *data, uint8_t **sei, size_t *size)
{
	blog(LOG_INFO, "alirtc_enc_sei ");
	// struct obs_qsv *obsqsv = data;

	// if (!obsqsv->context)
	// 	return false;

	// *sei = obsqsv->sei;
	// *size = obsqsv->sei_size;
	return true;
}

#define TEXT_RATE_CONTROL obs_module_text("RateControl")
#define TEXT_BITRATE obs_module_text("Bitrate")
#define TEXT_CUSTOM_BUF obs_module_text("CustomBufsize")
#define TEXT_BUF_SIZE obs_module_text("BufferSize")
#define TEXT_VFR obs_module_text("VFR")
#define TEXT_CRF obs_module_text("CRF")
#define TEXT_KEYINT_SEC obs_module_text("KeyframeIntervalSec")
#define TEXT_PRESET obs_module_text("CPUPreset")
#define TEXT_PROFILE obs_module_text("Profile")
#define TEXT_TUNE obs_module_text("Tune")
#define TEXT_NONE obs_module_text("None")
#define TEXT_X264_OPTS obs_module_text("EncoderOptions")

static const char * const x264_preset_names[] = { "ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow", "placebo", 0 };

static const char * const x264_tune_names[] = { "film", "animation", "grain", "stillimage", "psnr", "ssim", "fastdecode", "zerolatency", 0 };


static bool rate_control_modified(obs_properties_t *ppts, obs_property_t *p,
				  obs_data_t *settings)
{
	const char *rc = obs_data_get_string(settings, "rate_control");
	bool use_bufsize = obs_data_get_bool(settings, "use_bufsize");
	bool abr = astrcmpi(rc, "CBR") == 0 || astrcmpi(rc, "ABR") == 0;
	bool rc_crf = astrcmpi(rc, "CRF") == 0;

	p = obs_properties_get(ppts, "crf");
	obs_property_set_visible(p, !abr);

	p = obs_properties_get(ppts, "bitrate");
	obs_property_set_visible(p, !rc_crf);
	p = obs_properties_get(ppts, "use_bufsize");
	obs_property_set_visible(p, !rc_crf);
	p = obs_properties_get(ppts, "buffer_size");
	obs_property_set_visible(p, !rc_crf && use_bufsize);
	return true;
}

static bool use_bufsize_modified(obs_properties_t *ppts, obs_property_t *p,
				 obs_data_t *settings)
{
	bool use_bufsize = obs_data_get_bool(settings, "use_bufsize");
	const char *rc = obs_data_get_string(settings, "rate_control");
	bool rc_crf = astrcmpi(rc, "CRF") == 0;

	p = obs_properties_get(ppts, "buffer_size");
	obs_property_set_visible(p, use_bufsize && !rc_crf);
	return true;
}

static inline void add_strings(obs_property_t *list, const char *const *strings)
{
	while (*strings) {
		obs_property_list_add_string(list, *strings, *strings);
		strings++;
	}
}

static obs_properties_t *alirtc_enc_properties(void *unused)
{
	blog(LOG_INFO, "alirtc_enc_properties ");
UNUSED_PARAMETER(unused);

	obs_properties_t *props = obs_properties_create();
	// obs_property_t *list;
	obs_property_t *p;
	// obs_property_t *headers;

	// list = obs_properties_add_list(props, "rate_control", TEXT_RATE_CONTROL,
	// 			       OBS_COMBO_TYPE_LIST,
	// 			       OBS_COMBO_FORMAT_STRING);
	// obs_property_list_add_string(list, "CBR", "CBR");
	// obs_property_list_add_string(list, "ABR", "ABR");
	// obs_property_list_add_string(list, "VBR", "VBR");
	// obs_property_list_add_string(list, "CRF", "CRF");

	// obs_property_set_modified_callback(list, rate_control_modified);

	p = obs_properties_add_int(props, "bitrate", TEXT_BITRATE, 50, 10000000,
				   50);
// 	obs_property_int_set_suffix(p, " Kbps");

// 	p = obs_properties_add_bool(props, "use_bufsize", TEXT_CUSTOM_BUF);
// 	obs_property_set_modified_callback(p, use_bufsize_modified);
// 	obs_properties_add_int(props, "buffer_size", TEXT_BUF_SIZE, 0, 10000000,
// 			       1);

// 	obs_properties_add_int(props, "crf", TEXT_CRF, 0, 51, 1);

	p = obs_properties_add_int(props, "keyint_sec", TEXT_KEYINT_SEC, 0, 20,
				   1);
// 	obs_property_int_set_suffix(p, " s");

// 	list = obs_properties_add_list(props, "preset", TEXT_PRESET,
// 				       OBS_COMBO_TYPE_LIST,
// 				       OBS_COMBO_FORMAT_STRING);
// 	add_strings(list, x264_preset_names);

// 	list = obs_properties_add_list(props, "profile", TEXT_PROFILE,
// 				       OBS_COMBO_TYPE_LIST,
// 				       OBS_COMBO_FORMAT_STRING);
// 	obs_property_list_add_string(list, TEXT_NONE, "");
// 	obs_property_list_add_string(list, "baseline", "baseline");
// 	obs_property_list_add_string(list, "main", "main");
// 	obs_property_list_add_string(list, "high", "high");

// 	list = obs_properties_add_list(props, "tune", TEXT_TUNE,
// 				       OBS_COMBO_TYPE_LIST,
// 				       OBS_COMBO_FORMAT_STRING);
// 	obs_property_list_add_string(list, TEXT_NONE, "");
// 	add_strings(list, x264_tune_names);

// #ifdef ENABLE_VFR
// 	obs_properties_add_bool(props, "vfr", TEXT_VFR);
// #endif

// 	obs_properties_add_text(props, "x264opts", TEXT_X264_OPTS,
// 				OBS_TEXT_DEFAULT);

// 	headers = obs_properties_add_bool(props, "repeat_headers",
// 					  "repeat_headers");
// 	obs_property_set_visible(headers, false);

	return props;
}

static const char *alirtc_opus_getname(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "AliRtc Opus";
}

static const char *alirtc_aac_getname(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "AliRtc AAC";
}

static void  register_alirtc_qsv_encoder() {
	memset(&qsv_encoder_info, 0, sizeof(qsv_encoder_info));

	qsv_encoder_info.id = "alirtc_qsv";
	qsv_encoder_info.type = OBS_ENCODER_VIDEO;
	qsv_encoder_info.codec = "h264";
	qsv_encoder_info.get_name = alirtc_qsv_getname;	
	qsv_encoder_info.create = alirtc_qsv_enc_create;	
	qsv_encoder_info.destroy = alirtc_enc_destroy;	
	qsv_encoder_info.encode = alirtc_enc_encode;
	qsv_encoder_info.get_frame_size = alirtc_enc_frame_size;
	qsv_encoder_info.get_defaults = alirtc_enc_defaults;
	qsv_encoder_info.get_properties = alirtc_enc_properties;
	qsv_encoder_info.get_extra_data = alirtc_enc_extra_data;
	qsv_encoder_info.get_video_info = alirtc_video_info;
	qsv_encoder_info.update = alirtc_qsv_enc_update;
	qsv_encoder_info.get_sei_data = alirtc_enc_sei;
	obs_register_encoder(&qsv_encoder_info);
};

static void  register_alirtc_nv_encoder() {
	memset(&nvidia_encoder_info, 0, sizeof(nvidia_encoder_info));

	nvidia_encoder_info.id = "alirtc_nvidia";
	nvidia_encoder_info.type = OBS_ENCODER_VIDEO;
	nvidia_encoder_info.codec = "h264";
	nvidia_encoder_info.get_name = alirtc_nvidia_getname;	
	nvidia_encoder_info.create = alirtc_nvidia_enc_create;	
	nvidia_encoder_info.destroy = alirtc_enc_destroy;	
	nvidia_encoder_info.encode = alirtc_enc_encode;
	nvidia_encoder_info.get_frame_size = alirtc_enc_frame_size;
	nvidia_encoder_info.get_defaults = alirtc_enc_defaults;
	nvidia_encoder_info.get_properties = alirtc_enc_properties;
	nvidia_encoder_info.get_extra_data = alirtc_enc_extra_data;
	nvidia_encoder_info.get_video_info = alirtc_video_info;
	nvidia_encoder_info.update = alirtc_nvidia_enc_update;
	nvidia_encoder_info.get_sei_data = alirtc_enc_sei;
	obs_register_encoder(&nvidia_encoder_info);
};

static void  register_alirtc_x264_encoder() {
	memset(&x264_encoder_info, 0, sizeof(x264_encoder_info));
	x264_encoder_info.id = "alirtc_x264";
	x264_encoder_info.type = OBS_ENCODER_VIDEO;
	x264_encoder_info.codec = "h264";
	x264_encoder_info.get_name = alirtc_x264_getname;	
	x264_encoder_info.create = alirtc_x264_enc_create;	
	x264_encoder_info.destroy = alirtc_enc_destroy;	
	x264_encoder_info.encode = alirtc_enc_encode;
	x264_encoder_info.get_frame_size = alirtc_enc_frame_size;
	x264_encoder_info.get_defaults = alirtc_enc_defaults;
	x264_encoder_info.get_properties = alirtc_enc_properties;
	x264_encoder_info.get_extra_data = alirtc_enc_extra_data;
	x264_encoder_info.get_video_info = alirtc_video_info;
	x264_encoder_info.update = alirtc_x264_enc_update;
	x264_encoder_info.get_sei_data = alirtc_enc_sei;
	obs_register_encoder(&x264_encoder_info);
};

static void  register_alirtc_openh264_encoder() {
	memset(&openh264_encoder_info, 0, sizeof(openh264_encoder_info));
	openh264_encoder_info.id = "alirtc_openh264";
	openh264_encoder_info.type = OBS_ENCODER_VIDEO;
	openh264_encoder_info.codec = "h264";
	openh264_encoder_info.get_name = alirtc_openh264_getname;	
	openh264_encoder_info.create = alirtc_openh264_enc_create;	
	openh264_encoder_info.destroy = alirtc_enc_destroy;	
	openh264_encoder_info.encode = alirtc_enc_encode;
	openh264_encoder_info.get_frame_size = alirtc_enc_frame_size;
	openh264_encoder_info.get_defaults = alirtc_enc_defaults;
	openh264_encoder_info.get_properties = alirtc_enc_properties;
	openh264_encoder_info.get_extra_data = alirtc_enc_extra_data;
	openh264_encoder_info.get_video_info = alirtc_video_info;
	openh264_encoder_info.update = alirtc_openh264_enc_update;
	openh264_encoder_info.get_sei_data = alirtc_enc_sei;
	obs_register_encoder(&openh264_encoder_info);
};

static void  register_alirtc_x265_encoder() {
	memset(&x265_encoder_info, 0, sizeof(x265_encoder_info));
	x265_encoder_info.id = "alirtc_x265";
	x265_encoder_info.type = OBS_ENCODER_VIDEO;
	x265_encoder_info.codec = "h264";
	x265_encoder_info.get_name = alirtc_x265_getname;	
	x265_encoder_info.create = alirtc_x265_enc_create;	
	x265_encoder_info.destroy = alirtc_enc_destroy;	
	x265_encoder_info.encode = alirtc_enc_encode;
	x265_encoder_info.get_frame_size = alirtc_enc_frame_size;
	x265_encoder_info.get_defaults = alirtc_enc_defaults;
	x265_encoder_info.get_properties = alirtc_enc_properties;
	x265_encoder_info.get_extra_data = alirtc_enc_extra_data;
	x265_encoder_info.get_video_info = alirtc_video_info;
	x265_encoder_info.update = alirtc_x265_enc_update;
	x265_encoder_info.get_sei_data = alirtc_enc_sei;
	obs_register_encoder(&x265_encoder_info);
};

static void  register_alirtc_s265_encoder() {
	memset(&s265_encoder_info, 0, sizeof(s265_encoder_info));
	s265_encoder_info.id = "alirtc_s265";
	s265_encoder_info.type = OBS_ENCODER_VIDEO;
	s265_encoder_info.codec = "h264";
	s265_encoder_info.get_name = alirtc_s265_getname;	
	s265_encoder_info.create = alirtc_s265_enc_create;	
	s265_encoder_info.destroy = alirtc_enc_destroy;	
	s265_encoder_info.encode = alirtc_enc_encode;
	s265_encoder_info.get_frame_size = alirtc_enc_frame_size;
	s265_encoder_info.get_defaults = alirtc_enc_defaults;
	s265_encoder_info.get_properties = alirtc_enc_properties;
	s265_encoder_info.get_extra_data = alirtc_enc_extra_data;
	s265_encoder_info.get_video_info = alirtc_video_info;
	s265_encoder_info.update = alirtc_s265_enc_update;
	s265_encoder_info.get_sei_data = alirtc_enc_sei;
	obs_register_encoder(&s265_encoder_info);
};

static void  register_alirtc_opus_encoder() {
	memset(&opus_encoder_info, 0, sizeof(opus_encoder_info));
	opus_encoder_info.id = "alirtc_opus";
	opus_encoder_info.type = OBS_ENCODER_AUDIO;
	opus_encoder_info.codec = "opus";
	opus_encoder_info.get_name = alirtc_opus_getname;	
	opus_encoder_info.create = alirtc_opus_enc_create;	
	opus_encoder_info.destroy = alirtc_enc_destroy;	
	opus_encoder_info.encode = alirtc_enc_encode;
	opus_encoder_info.get_frame_size = alirtc_enc_frame_size;
	opus_encoder_info.get_defaults = alirtc_enc_defaults;
	opus_encoder_info.get_properties = NULL;
	opus_encoder_info.get_extra_data = alirtc_enc_extra_data;
	opus_encoder_info.get_audio_info = alirtc_enc_audio_info;
	obs_register_encoder(&opus_encoder_info);
};

static void  register_alirtc_aac_encoder() {
	memset(&aac_encoder_info, 0, sizeof(aac_encoder_info));
	aac_encoder_info.id = "alirtc_aac";
	aac_encoder_info.type = OBS_ENCODER_AUDIO;
	aac_encoder_info.codec = "aac";
	aac_encoder_info.get_name = alirtc_aac_getname;	
	aac_encoder_info.create = alirtc_aac_enc_create;	
	aac_encoder_info.destroy = alirtc_enc_destroy;	
	aac_encoder_info.encode = alirtc_enc_encode;
	aac_encoder_info.get_frame_size = alirtc_enc_frame_size;
	aac_encoder_info.get_defaults = alirtc_enc_defaults;
	aac_encoder_info.get_properties = NULL;
	aac_encoder_info.get_extra_data = alirtc_enc_extra_data;
	aac_encoder_info.get_audio_info = alirtc_enc_audio_info;
	obs_register_encoder(&aac_encoder_info);
};

void register_alirtc_encoder() {

	AlivcLivePusher * pusher = obs_alirtc_get_instace();
	if (pusher) {
		AlivcVideoCodecManufacturerList  codecManufacturerList;

		pusher->getVideoSupportEncoderManufacturerList(codecManufacturerList);
		for(int i = 0 ; i < codecManufacturerList.codecCount; i++) {
			if(codecManufacturerList.codecType[i] == AlivcVideoCodecManufactureropenH264) {
				register_alirtc_openh264_encoder();
			} else if (codecManufacturerList.codecType[i] == AlivcVideoCodecManufacturerX264) {
				register_alirtc_x264_encoder();	
			} else if (codecManufacturerList.codecType[i] == AlivcVideoCodecManufacturerX265) {
				register_alirtc_x265_encoder();	
			} else if (codecManufacturerList.codecType[i] == AlivcVideoCodecManufacturerQsvIntel) {
				register_alirtc_qsv_encoder();
			} else if (codecManufacturerList.codecType[i] == AlivcVideoCodecManufacturerNvidia) {
				register_alirtc_nv_encoder();
			} else if (codecManufacturerList.codecType[i] == AlivcVideoCodecManufacturerS265) {
				register_alirtc_s265_encoder();
			}
			obs_alirtc_destory_instace();    
		}
	}
	register_alirtc_opus_encoder();
	register_alirtc_aac_encoder();			
}
