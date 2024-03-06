#include <jni.h>
#include <random>
#include <ctime>

#include <tunmode/tunmode.hpp>

extern "C"
JNIEXPORT void JNICALL
Java_git_gxosty_tunmode_interceptor_services_TunModeService_tunnelOpenNative(JNIEnv* env, jclass cls, int fd, jstring dns_address)
{
	tunmode::params::dns_address.s_addr = 0;
	tunmode::params::tun = fd;
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
	srand(time(NULL));
	tunmode::set_jvm(vm);
	return JNI_VERSION_1_6;
}