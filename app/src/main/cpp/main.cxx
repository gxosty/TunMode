#include <jni.h>
#include <random>
#include <ctime>
#include <cstring>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <tunmode/tunmode.hpp>

extern "C"
JNIEXPORT void JNICALL
Java_git_gxosty_tunmode_interceptor_services_TunModeService_tunnelOpenNative(JNIEnv* env, jclass cls, int fd, jstring net_iface, jstring dns_address)
{
	tunmode::params::tun = fd;
	tunmode::params::dns_address.s_addr = 0;

	const char* str_net_iface = env->GetStringUTFChars(net_iface, NULL);

	struct ifreq ifr;
	int if_fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, str_net_iface, IFNAMSIZ);
	ioctl(if_fd, SIOCGIFADDR, &ifr);

	close(if_fd);
	tunmode::params::net_iface = ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr;

	env->ReleaseStringUTFChars(net_iface, str_net_iface);

	if (dns_address)
	{
		const char* str_dns_address = env->GetStringUTFChars(dns_address, NULL);
		inet_pton(AF_INET, str_dns_address, &tunmode::params::dns_address);
		env->ReleaseStringUTFChars(dns_address, str_dns_address);
	}

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