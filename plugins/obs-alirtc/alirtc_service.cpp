#include "alirtc_service.h"

struct ALIRTCService {
	std::string server;
	std::string bearer_token;

	ALIRTCService(obs_data_t *settings, obs_service_t *service);

	void Update(obs_data_t *settings);
	static obs_properties_t *Properties();
	static void ApplyEncoderSettings(obs_data_t *video_settings,
					 obs_data_t *audio_settings);
	bool CanTryToConnect();
	const char *GetConnectInfo(enum obs_service_connect_info type);
};

const char *audio_codecs[MAX_CODECS] = {"opus","aac"};
const char *video_codecs[MAX_CODECS] = {"h264", "hevc"};


ALIRTCService::ALIRTCService(obs_data_t *settings, obs_service_t *service) {
	blog(LOG_INFO, "ALIRTCService ctor");
}

void ALIRTCService::Update(obs_data_t *settings) {
	blog(LOG_INFO, "ALIRTCService Update");
}

void ALIRTCService::ApplyEncoderSettings(obs_data_t *video_settings,
                    obs_data_t *audio_settings) {
	blog(LOG_INFO, "ALIRTCService ApplyEncoderSettings");
}

bool ALIRTCService::CanTryToConnect() {
	blog(LOG_INFO, "ALIRTCService CanTryToConnect");
	return true;
}

const char * ALIRTCService::GetConnectInfo(enum obs_service_connect_info type) {
	blog(LOG_INFO, "ALIRTCService GetConnectInfo");
	return NULL;
}

obs_properties_t * ALIRTCService::Properties() {
	blog(LOG_INFO, "ALIRTCService Properties");
	return NULL;
}

void register_alirtc_service() {
    struct obs_service_info info = {};

	info.id = "alirtc_custom";
	info.get_name = [](void *) -> const char * {
		return obs_module_text("Service.Name");
	};
	info.create = [](obs_data_t *settings,
			 obs_service_t *service) -> void * {
		return new ALIRTCService(settings, service);
	};
	info.destroy = [](void *priv_data) {
		delete static_cast<ALIRTCService *>(priv_data);
	};
	info.update = [](void *priv_data, obs_data_t *settings) {
		static_cast<ALIRTCService *>(priv_data)->Update(settings);
	};
	info.get_properties = [](void *) -> obs_properties_t * {
		return ALIRTCService::Properties();
	};
	info.get_protocol = [](void *) -> const char * {
		return "ALIRTC";
	};
	info.get_url = [](void *priv_data) -> const char * {
		return static_cast<ALIRTCService *>(priv_data)->server.c_str();
	};
	info.get_output_type = [](void *) -> const char * {
		return "alirtc_output";
	};
	info.apply_encoder_settings = [](void *, obs_data_t *video_settings,
					 obs_data_t *audio_settings) {
		ALIRTCService::ApplyEncoderSettings(video_settings,
						  audio_settings);
	};
	info.get_supported_video_codecs = [](void *) -> const char ** {
		return video_codecs;
	};
	info.get_supported_audio_codecs = [](void *) -> const char ** {
		return audio_codecs;
	};
	info.can_try_to_connect = [](void *priv_data) -> bool {
		return static_cast<ALIRTCService *>(priv_data)->CanTryToConnect();
	};
	info.get_connect_info = [](void *priv_data,
				   uint32_t type) -> const char * {
		return static_cast<ALIRTCService *>(priv_data)->GetConnectInfo(
			(enum obs_service_connect_info)type);
	};
	obs_register_service(&info);
}
