#include <obs-module.h>
#include "alirtc_output.h"
#include "alirtc_service.h"
#include "alirtc_encoder.h"


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-alirtc", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "OBS AliRtc module";
}

bool obs_module_load()
{
    register_alirtc_output();
    register_alirtc_encoder();
    register_alirtc_service();    
	return true;
}
