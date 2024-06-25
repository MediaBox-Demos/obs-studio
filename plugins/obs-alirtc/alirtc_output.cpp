#include "alirtc_output.h"
#include "alirtc_common.h"
#include <thread>
#include "util/platform.h"

#define do_log(level, format, ...)                              \
	blog(level, "[obs-alirtc] [alirtc_output: '%s'] " format, \
	     obs_output_get_name(output_), ##__VA_ARGS__)


static uint64_t os_gettime_ms(void)
{
	return os_gettime_ns() / 100000;
}

class AlirtcOutput : public AlivcLivePushInfoListener,
		     public AlivcLivePushErrorListener,
		     public AlivcLivePushNetworkListener,
			 public AlivcLivePlayInfoListener {
public:
	AlirtcOutput(obs_data_t *settings, obs_output_t *output);
	~AlirtcOutput();

	bool Start();
	void Stop(bool signal = true);
	void Data(struct encoder_packet *packet);
	size_t GetTotalBytes();
	int GetConnectTime();

	int StartPlay(const char *url, void *hwnd, int width, int height);
	int StopPlay();

	void SetCallback(struct obs_output_callback* callback) {
		callback_ = callback;
	}

	void Update(obs_data_t *settings);

	void InsertVideoFrame(struct video_data *frame);

	void InsertAudioFrame(struct audio_data *frames);
	virtual void onPushStarted(AlivcLivePusher *pusher) override;

	virtual void onPushStopped(AlivcLivePusher *pusher) override;
	virtual void onNetworkPoor(AlivcLivePusher *livePusher) override;
	virtual void onNetworkRecovery(AlivcLivePusher *livePusher) override;
	virtual void onReconnectStart(AlivcLivePusher *livePusher) override;
	virtual void onConnectionLost(AlivcLivePusher *livePusher) override;
	virtual void onReconnectFail(AlivcLivePusher *livePusher) override;
	virtual void onReconnectSucceed(AlivcLivePusher *livePusher) override;
	virtual void onSendDataTimeout(AlivcLivePusher *livePusher) override;
	virtual void onConnectFail(AlivcLivePusher *livePusher) override;
	virtual void onPushURLAuthenticationOverdue(AlivcLivePusher *livePusher) override;
	virtual void onSendMessage(AlivcLivePusher *livePusher) override;
	virtual void onPacketsLost(AlivcLivePusher *livePusher) override;
	virtual void onConnectionStatusChange(int status, int reason) override;
	virtual void onNetworkQualityChange(AlivcLiveNetworkQuality quality) override;
	virtual void onSystemError(AlivcLivePusher* livePusher, AlivcLivePushError error) override;
	virtual void onSDKError(AlivcLivePusher* livePusher, AlivcLivePushError error) override;

	virtual void onPlayStarted(AlivcLivePlayer* player) override;
	virtual void onPlayStopped(AlivcLivePlayer* player) override;
	virtual void onError(AlivcLivePlayer* player, AlivcLivePlayError code, const char* msg) override;

private:

	void ReleasePusher();

	void CreatePusher();

	void GetPusherConfig();

	void StartThread();

	void StartObsCapture();

private:
	obs_output_t *output_;
	AlivcLivePusher * pusher_{nullptr};

	std::mutex start_stop_mutex;
	std::thread start_stop_thread;

	AlivcLivePushConfig config_;
	std::string url_;

	bool is_connected_{false};

	bool is_capture_started_{false};

	uint8_t *video_buffer_{nullptr};
	int32_t video_buffer_length_{0};

	int audio_channel_{1};
	int audio_sample_rate_{48000};
	int key_int_sec_{2};

	bool dump_audio_{false};
	FILE *fp_audio_ { nullptr};

	bool first_audio_capture_{false};

	int audio_bitrate_{1500};
	int video_bitrate_{1500};
	int fps_num_{0};
	std::string video_encoder_name_;	
	std::string audio_encoder_name_;
	int width_{720};	
	int height_{1280};

	struct obs_output_callback* callback_;

	AlivcLivePlayer *player_{nullptr};
};
AlirtcOutput::AlirtcOutput(obs_data_t *settings, obs_output_t *output) {

	output_ = output;
	do_log(LOG_INFO, "AlirtcOutput ctor");
}

AlirtcOutput::~AlirtcOutput() {
	do_log(LOG_INFO, "AlirtcOutput ~dtor");
	Stop();
}

void AlirtcOutput::ReleasePusher()
{
	if (is_connected_ && pusher_) {
		pusher_->stopPush();
	}

	if (pusher_) {
		AlivcLivePusher::destroy();
		pusher_ = nullptr;
	}
	if (video_buffer_) {
		bfree(video_buffer_);
		video_buffer_ = nullptr;
	}

	if (dump_audio_ && fp_audio_) {
		fclose(fp_audio_);
		fp_audio_ = nullptr;
	}

	if (is_capture_started_) {
		obs_output_end_data_capture(output_);
	}
}

void AlirtcOutput::StartThread()
{
	do_log(LOG_INFO, "AlirtcOutput StartThread");

	if (!pusher_) {
		return;
	}

}

void AlirtcOutput::GetPusherConfig() {
	video_t *video = obs_output_video(output_);
	const struct video_output_info *voi = video_output_get_info(video);
	obs_data_t *settings;
	settings = obs_output_get_settings(output_);

	struct obs_audio_info aoi;

	if (!obs_get_audio_info(&aoi)) {
		return;
	}
	audio_channel_ = get_audio_channels(aoi.speakers);

	audio_sample_rate_ = aoi.samples_per_sec;

	config_.mTargetVideoBitrate = video_bitrate_;
	if (audio_channel_ > 1) {
		config_.audioProfile = AlivcLiveStereoHighQualityMode;
	} else {
		config_.audioProfile = AlivcLiveHighQualityMode;
	}

	config_.audioProfile = AlivcLiveStereoHighQualityMode;
	config_.height = height_;
	config_.width = width_;
	if (config_.height > config_.width) {
		config_.mPreviewOrientation =
			AlivcLivePreviewOrientationPortRait;
	} else {
		config_.mPreviewOrientation =
			AlivcLivePreviewOrientationLandscape;
	}
	if (voi->fps_num > 30) {
		config_.frameRate = AlivcLiveFrameRateFps60;
	} else if (voi->fps_num > 25) {
		config_.frameRate = AlivcLiveFrameRateFps30;
	} else if (voi->fps_num > 20) {
		config_.frameRate = AlivcLiveFrameRateFps25;
	} else if (voi->fps_num > 15) {
		config_.frameRate = AlivcLiveFrameRateFps20;
	} else if (voi->fps_num > 10) {
		config_.frameRate = AlivcLiveFrameRateFps15;
	} else if (voi->fps_num > 5) {
		config_.frameRate = AlivcLiveFrameRateFps10;
	} else {
		config_.frameRate = AlivcLiveFrameRateFps30;
	}
	config_.mVideoEncodeGop = (AlivcLiveVideoEncodeGopEnum)key_int_sec_;

	config_.mResolution = AlivcLiveResolutionSelfDefine;
	config_.externMainStream = true;
	config_.mLivePushMode = AlivcLiveInteractiveMode;

	if (audio_encoder_name_.find("aac") != std::string::npos) {
		config_.mAudioCodecEncodeType = AlivcAudioCodecAAC;
	}
}

void AlirtcOutput::CreatePusher() {
	GetPusherConfig();
	if (!pusher_) {
		pusher_ = AlivcLivePusher::Create("");
	}
	pusher_->init(config_);

	pusher_->setLivePushErrorListener(this);
	pusher_->setLivePushInfoListener(this);
	pusher_->setLivePushNetworkListener(this);
	pusher_->setExternalVideoSource( true, AlivcLiveVideoTrackCamera);
	pusher_->setExternalAudioSource(true, audio_sample_rate_,
					audio_channel_);
	pusher_->setParameter("{\"audio\":{\"user_specified_audio_external_buf_size\":6}}");

	video_buffer_length_ = static_cast<size_t>(config_.height) * config_.width * 3 / 2;
	video_buffer_ = (uint8_t *)bzalloc(video_buffer_length_);

	if (dump_audio_) {
		fp_audio_ = fopen("obs_audio_capture.pcm", "wb+");
	}
}

bool AlirtcOutput::Start() {
	do_log(LOG_INFO, "AlirtcOutput Start");
	std::lock_guard<std::mutex> l(start_stop_mutex);

	CreatePusher();
	do_log(LOG_INFO, "AlirtcOutput Start url %s", url_.c_str());

	if (video_encoder_name_.find("x264") != std::string::npos) {
		pusher_->setVideoEncoderCodecManufacturerConfig(
			AlivcVideoCodecManufacturerX264);
	} else if (video_encoder_name_.find("x265") != std::string::npos) {
		pusher_->setVideoEncoderCodecManufacturerConfig(
			AlivcVideoCodecManufacturerX265);
	} else if (video_encoder_name_.find("s265") != std::string::npos) {
		pusher_->setVideoEncoderCodecManufacturerConfig(
			AlivcVideoCodecManufacturerS265);
	} else if (video_encoder_name_.find("nvidia") != std::string::npos) {
		pusher_->setVideoEncoderCodecManufacturerConfig(
			AlivcVideoCodecManufacturerNvidia);
	} else if (video_encoder_name_.find("qsv") != std::string::npos) {
		pusher_->setVideoEncoderCodecManufacturerConfig(
			AlivcVideoCodecManufacturerQsvIntel);
	}  else if (video_encoder_name_.find("s264") != std::string::npos) {
		pusher_->setVideoEncoderCodecManufacturerConfig(
			AlivcVideoCodecManufacturerS264);
	} else {
		pusher_->setVideoEncoderCodecManufacturerConfig(
			AlivcVideoCodecManufacturerX264);
	}
	pusher_->startPush(url_.c_str());

	StartObsCapture();
	return true;
}

void AlirtcOutput::StartObsCapture() {
	struct audio_convert_info aci {};
	aci.format = AUDIO_FORMAT_16BIT;
	if (!obs_output_can_begin_data_capture(output_, 0))
		return;

	obs_output_set_video_conversion(output_, NULL);
	obs_output_set_audio_conversion(output_, &aci);
	obs_output_begin_data_capture(output_, 0);
	
	is_capture_started_ = true;
}

void AlirtcOutput::Stop(bool signal) {
	do_log(LOG_INFO, "AlirtcOutput Stop");
	ReleasePusher();
}

void AlirtcOutput::Data(struct encoder_packet *packet) {
	do_log(LOG_INFO, "AlirtcOutput Data");
}

size_t AlirtcOutput::GetTotalBytes() {
	do_log(LOG_INFO, "AlirtcOutput GetTotalBytes");
	return 0;
}

int AlirtcOutput::GetConnectTime() {
	do_log(LOG_INFO, "AlirtcOutput GetConnectTime");
	return 0;
}

int AlirtcOutput::StartPlay(const char *url, void *hwnd, int width, int height) {
	player_ = AlivcLivePlayer::Create("");
	if (player_) {
		AlivcLivePlayConfig config;
		player_->setupWithConfig(config);
		player_->startPlay(url);
		player_->setPlayView(hwnd, width, height);
	}
	return 0;
}

int AlirtcOutput::StopPlay() {
	if (player_) {
		player_->stopPlay();
		player_->destroy();
		player_ = nullptr;
	}
	return 0;
}

void AlirtcOutput::Update(obs_data_t *settings) {
	do_log(LOG_INFO, "AlirtcOutput Update");

	if (obs_data_get_string(settings, "url") != "") {
		url_ = obs_data_get_string(settings, "url");
	}

	if ((int)obs_data_get_int(settings, "video_bitrate") != 0) {
		video_bitrate_ =
			(int)obs_data_get_int(settings, "video_bitrate");
	}
	if ((int)obs_data_get_int(settings, "audio_bitrate") != 0) {
		audio_bitrate_ =
			(int)obs_data_get_int(settings, "audio_bitrate");
	}

	if ((int)obs_data_get_int(settings, "fps_num") != 0) {
		fps_num_ =
			(int)obs_data_get_int(settings, "fps_num");
	}

	if ((int)obs_data_get_int(settings, "audio_channels") != 0) {
		audio_channel_ =
			(int)obs_data_get_int(settings, "audio_channels");
	}

	if ((int)obs_data_get_int(settings, "audio_sample_rate") != 0) {
		audio_sample_rate_ =
			(int)obs_data_get_int(settings, "audio_sample_rate");
	}
	if ((int)obs_data_get_int(settings, "keyint_sec") != 0) {
		key_int_sec_ =
			(int)obs_data_get_int(settings, "keyint_sec");
	}
	
	if ((int)obs_data_get_int(settings, "width") != 0) {
		width_ = (int)obs_data_get_int(settings, "width");
	}

	if ((int)obs_data_get_int(settings, "height") != 0) {
		height_ = (int)obs_data_get_int(settings, "height");
	}

	if (obs_data_get_string(settings, "video_codec_name") != "") {
		video_encoder_name_ =
			obs_data_get_string(settings, "video_codec_name");
	}

	if (obs_data_get_string(settings, "audio_codec_name") != "") {
		audio_encoder_name_ =
			obs_data_get_string(settings, "audio_codec_name");
	}

	
}

void AlirtcOutput::InsertVideoFrame(struct video_data *frame) {
	if (pusher_) {
		int height = (int)obs_output_get_height(output_);
		int width = (int)obs_output_get_width(output_);
		int length = width * height * 3 / 2;
		if (length != video_buffer_length_) {
			if (video_buffer_) {
				bfree(video_buffer_);
			}

			video_buffer_ = (uint8_t *)bzalloc(length);
			if (!video_buffer_) {
				return ;
			}
			video_buffer_length_ = length;
		}

		memcpy(video_buffer_, frame->data[0],
		       static_cast<size_t>(width) * height);
		memcpy(video_buffer_ + width * height, frame->data[1],
		       static_cast<size_t>(width) * height / 4);
		memcpy(video_buffer_ + width * height + (width * height / 4),
		       frame->data[2],
		       static_cast<size_t>(width) * height / 4);

		AlivcLiveVideoRawData frameRaw;
		frameRaw.format = AlivcLiveVideoFormatI420;
		frameRaw.type = AlivcLiveBufferTypeRawData;
		frameRaw.dataLength = video_buffer_length_;
		frameRaw.dataPtr = video_buffer_;
		frameRaw.dataYPtr = video_buffer_;
		frameRaw.dataUPtr = video_buffer_ + width * height;
		frameRaw.dataVPtr = video_buffer_ + width * height + (width * height / 4);
		frameRaw.strideY = frame->linesize[0];
		frameRaw.strideU = frame->linesize[1];
		frameRaw.strideV = frame->linesize[2];
		frameRaw.height = height;
		frameRaw.width = width;
		pusher_->pushExternalVideoFrame(frameRaw,
						AlivcLiveVideoTrackCamera);
	}
}

void AlirtcOutput::InsertAudioFrame(struct audio_data *frames)
{
	if (!first_audio_capture_) {
		do_log(LOG_INFO, "AlirtcOutput InsertAudioFrame first pcm %d",
		     frames->frames);
		first_audio_capture_ = true;
	}

	if (pusher_) {
		pusher_->pushExternalAudioFrameRawData(frames->data[0], frames->frames * 2 * audio_channel_);
	}

	if (dump_audio_ && fp_audio_) {
		fwrite(frames->data[0], 1,
		       static_cast<size_t>(frames->frames) * 2 * audio_channel_,
		       fp_audio_);
	}
}

static void receive_video(void *param, struct video_data *frame)
{
	// blog(LOG_INFO, "AlirtcOutput receive_video");
	AlirtcOutput *output = (AlirtcOutput *)param;

	output->InsertVideoFrame(frame);
}

static void receive_audio(void *data, struct audio_data *frames)
{
	// blog(LOG_INFO, "AlirtcOutput receive_audio time:%lld, frames:%d", os_gettime_ms(),
	//      frames->frames);
	AlirtcOutput *output = (AlirtcOutput *)data;

	output->InsertAudioFrame(frames);

}

void AlirtcOutput::onPushStarted(AlivcLivePusher *pusher) {
	is_connected_ = true;
	if (callback_) {
		callback_->on_push_started(callback_->data);
	}
}

void AlirtcOutput::onPushStopped(AlivcLivePusher *pusher) {
	is_connected_ = false;
	if (callback_) {
		callback_->on_push_stoped(callback_->data);
	}
}

void AlirtcOutput::onNetworkPoor(AlivcLivePusher *livePusher) {

}
void AlirtcOutput::onNetworkRecovery(AlivcLivePusher *livePusher) {

}
void AlirtcOutput::onReconnectStart(AlivcLivePusher *livePusher) {
	if (callback_) {
		callback_->on_reconnect_start(callback_->data);
	}
}
void AlirtcOutput::onConnectionLost(AlivcLivePusher *livePusher) {
	if (callback_) {
		callback_->on_connect_lost(callback_->data);
	}
}
void AlirtcOutput::onReconnectFail(AlivcLivePusher *livePusher) {
	if (callback_) {
		callback_->on_reconnect_failed(callback_->data);
	}
}
void AlirtcOutput::onReconnectSucceed(AlivcLivePusher *livePusher) {
	if (callback_) {
		callback_->on_reconnect_sucess(callback_->data);
	}
}
void AlirtcOutput::onSendDataTimeout(AlivcLivePusher *livePusher) {

}

void AlirtcOutput::onConnectFail(AlivcLivePusher *livePusher) {
	if (callback_) {
		callback_->on_connect_fail(callback_->data);
	}
}

void AlirtcOutput::onPushURLAuthenticationOverdue(AlivcLivePusher *livePusher)
{

}
void AlirtcOutput::onSendMessage(AlivcLivePusher *livePusher) {

}

void AlirtcOutput::onPacketsLost(AlivcLivePusher *livePusher) {

}

void AlirtcOutput::onConnectionStatusChange(int status, int reason) {

}
void AlirtcOutput::onNetworkQualityChange(AlivcLiveNetworkQuality quality) {

}

void AlirtcOutput::onSystemError(AlivcLivePusher* livePusher, AlivcLivePushError error) {
	if (callback_) {
		callback_->on_error(callback_->data, (int)error.code);
	}
}

void AlirtcOutput::onSDKError(AlivcLivePusher* livePusher, AlivcLivePushError error) {
	if (callback_) {
		callback_->on_error(callback_->data, (int)error.code);
	}
}

void AlirtcOutput::onPlayStarted(AlivcLivePlayer* player) {
	if (callback_) {
		callback_->on_play_started(callback_->data);
	}
}

void AlirtcOutput::onPlayStopped(AlivcLivePlayer* player) {
	if (callback_) {
		callback_->on_play_stop(callback_->data);
	}
}

void AlirtcOutput::onError(AlivcLivePlayer* player, AlivcLivePlayError code, const char* msg) {
	if (callback_) {
		callback_->on_play_error(callback_->data, (int)code);
	}
}

void register_alirtc_output() {
	const uint32_t base_flags = OBS_OUTPUT_AV | OBS_OUTPUT_SERVICE;

	struct obs_output_info info = {};
	info.id = "alirtc_output";
	info.flags = OBS_OUTPUT_AV | base_flags;
	info.get_name = [](void *) -> const char * {
		return obs_module_text("Output.Name");
	};
	info.create = [](obs_data_t *settings, obs_output_t *output) -> void * {
		return new AlirtcOutput(settings, output);
	};
	info.destroy = [](void *priv_data) {
		delete static_cast<AlirtcOutput *>(priv_data);
	};
	info.start = [](void *priv_data) -> bool {
		return static_cast<AlirtcOutput *>(priv_data)->Start();
	};
	info.stop = [](void *priv_data, uint64_t) {
		static_cast<AlirtcOutput *>(priv_data)->Stop();
	};
	info.encoded_packet = [](void *priv_data,
				 struct encoder_packet *packet) {
		static_cast<AlirtcOutput *>(priv_data)->Data(packet);
	};
	info.update = [](void *priv_data,
				 obs_data_t *settings) {
					static_cast<AlirtcOutput *>(priv_data)->Update(settings);
				 };
	info.get_defaults = [](obs_data_t *) {
	};
	info.get_properties = [](void *) -> obs_properties_t * {
		return obs_properties_create();
	};
	info.get_total_bytes = [](void *priv_data) -> uint64_t {
		return (uint64_t) static_cast<AlirtcOutput *>(priv_data)
			->GetTotalBytes();
	};
	info.get_connect_time_ms = [](void *priv_data) -> int {
		return static_cast<AlirtcOutput *>(priv_data)->GetConnectTime();
	};
	info.set_output_callback = [](void *priv_data, struct obs_output_callback* callback) -> void {
		static_cast<AlirtcOutput *>(priv_data)->SetCallback(callback); 
	};

	info.start_play_remote_url = [](void *priv_data, const char * url, void * hwnd, int width, int height) -> void {
		static_cast<AlirtcOutput *>(priv_data)
					->StartPlay(url, hwnd, width, height);
	};
	info.stop_play_remote_url = [](void *priv_data) -> void {
		static_cast<AlirtcOutput *>(priv_data)
					->StopPlay();
	};
	
	info.raw_video = receive_video,
	info.raw_audio = receive_audio,
	info.encoded_video_codecs = "h264;hevc";
	info.encoded_audio_codecs = "opus;aac";
	info.protocols = "ALIRTC";

	obs_register_output(&info);

	info.id = "alirtc_output_video";
	info.flags = OBS_OUTPUT_VIDEO | base_flags;
	obs_register_output(&info);

	info.id = "alirtc_output_audio";
	info.flags = OBS_OUTPUT_AUDIO | base_flags;
	obs_register_output(&info);
}
