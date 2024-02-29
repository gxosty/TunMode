#include <jni.h>

#include <tunmode/tunmode.hpp>

extern "C"
JNIEXPORT void JNICALL
Java_git_gxosty_tunmode_interceptor_services_TunModeService_tunnelOpenNative(JNIEnv* env, jclass cls, int fd, jstring dns_address, jstring network_interface)
{
	tunmode::params::tun = fd;
	// tunmode::params::tun_addr = tun_address;
	tunmode::params::tun_addr.s_addr = 0;
	tunmode::open_tunnel();
}

extern "C"
JNIEXPORT void JNICALL
Java_git_gxosty_tunmode_interceptor_services_TunModeService_tunnelCloseNative(JNIEnv* env, jclass cls)
{
	tunmode::close_tunnel();
}

extern "C"
JNIEXPORT void JNICALL
Java_git_gxosty_tunmode_interceptor_services_TunModeService_setupNative(JNIEnv* env, jclass cls, jobject TunModeService_object)
{
	tunmode::initialize(env, TunModeService_object);
}

extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved)
{
	tunmode::set_jvm(vm);
	return JNI_VERSION_1_6;
}