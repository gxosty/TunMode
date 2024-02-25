#pragma once

#include <jni.h>

#include <string>

namespace tunmode
{
	namespace params
	{
		extern int tun_fd;
		extern jobject TunModeService_object;
	}

	void set_jvm(JavaVM* jvm);
	void initialize(JNIEnv* env, jobject TunModeService_object);
	int get_jni_env(JNIEnv** env);
	void open_tunnel();
	void close_tunnel();
}