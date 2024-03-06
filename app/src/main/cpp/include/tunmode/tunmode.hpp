#pragma once

#include "socket/tunsocket.hpp"

#include <jni.h>
#include <netinet/in.h>
#include <atomic>
#include <string>

namespace tunmode
{
	namespace params
	{
		extern TunSocket tun;
		extern in_addr net_iface;
		extern in_addr dns_address;
		extern jobject TunModeService_object;
		extern std::atomic<bool> stop_flag;
	}

	void set_jvm(JavaVM* jvm);
	void initialize(JNIEnv* env, jobject TunModeService_object);
	int get_jni_env(JNIEnv** env);
	void open_tunnel();
	void close_tunnel();
}