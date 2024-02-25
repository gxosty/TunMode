#pragma once

#ifndef RELEASE_MODE
	#include <android/log.h>

	#define LOGD(_tag_, ...) ((void)__android_log_print(ANDROID_LOG_DEBUG, _tag_, __VA_ARGS__))
	#define LOGE(_tag_, ...) ((void)__android_log_print(ANDROID_LOG_ERROR, _tag_, __VA_ARGS__))
	#define LOGI(_tag_, ...) ((void)__android_log_print(ANDROID_LOG_WARN,  _tag_, __VA_ARGS__))
	#define LOGW(_tag_, ...) ((void)__android_log_print(ANDROID_LOG_INFO,  _tag_, __VA_ARGS__))
#else
	#define LOGD(...)
	#define LOGE(...)
	#define LOGI(...)
	#define LOGW(...)
#endif // RELEASE_MODE