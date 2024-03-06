#include <tunmode/tunmode.hpp>
#include <tunmode/socket/sessionsocket.hpp>
#include <tunmode/manager/tcpmanager.hpp>
#include <tunmode/manager/udpmanager.hpp>
#include <misc/logger.hpp>

#include <future>
#include <string>
#include <vector>
#include <thread>

#include <poll.h>
#include <unistd.h>

namespace tunmode
{
	namespace params
	{
		JavaVM* jvm;
		TunSocket tun;
		in_addr dns_address;
		jobject TunModeService_object;
		std::atomic<bool> stop_flag;
		
		std::promise<void> tunnel_promise;
		std::atomic<int> thread_count;
	}

	TCPManager tcp_session_manager;
	UDPManager udp_session_manager;

	void set_jvm(JavaVM* jvm)
	{
		params::jvm = jvm;
	}

	void initialize(JNIEnv* env, jobject TunModeService_object)
	{
		params::tun = 0;
		SessionSocket::tun = &params::tun;
		params::TunModeService_object = env->NewGlobalRef(TunModeService_object);


		params::stop_flag.store(false);
		params::thread_count.store(0);
	}

	int get_jni_env(JNIEnv** env)
	{
		int status = params::jvm->GetEnv((void**)env, JNI_VERSION_1_6);

		if (status == JNI_EDETACHED) {
			if (params::jvm->AttachCurrentThread(env, nullptr) != 0) {
				LOGE(TAG, "Couldn't attach thread");
				return 2; // Failed to attach
			}
			LOGI(TAG, "Attached JNIEnv*");
			return 1; // Attached, need detach
		}

		return 0; // Already attached
	}

	void _thread_start()
	{
		params::thread_count++;
	}

	void _thread_stop()
	{
		int val = params::thread_count.fetch_sub(1) - 1;

		if (val == 0)
		{
			params::tunnel_promise.set_value();
		}
	}

	void _tunnel_loop()
	{
		_thread_start();

		while (!params::stop_flag.load())
		{
			int revents = 0;
			int ret = params::tun.poll(2000, revents);

			if (ret == -1)
			{
				LOGE(TAG, "Poll Error");
				break;
			}
			else if (ret == 0)
			{
				continue;    // Timeout reached
			}
			else
			{
				if (revents & POLLIN)
				{
					Packet packet;
					params::tun > packet;

					switch (packet.get_protocol())
					{
					case TUNMODE_PROTOCOL_TCP:
						tcp_session_manager.handle_packet(packet);
						break;

					case TUNMODE_PROTOCOL_UDP:
						udp_session_manager.handle_packet(packet);
						break;

					default:
						break;
					}
				}
				else
				{
					params::stop_flag.store(true);
				}
			}
		}

		_thread_stop();
	}

	void _run_loops()
	{
		std::thread tunnel_loop_thread(_tunnel_loop);
		tunnel_loop_thread.detach();
	}

	void _cleanup()
	{

	}

	void _tunnel_closed()
	{
		params::tun.close();
		params::tun = 0;

		if (params::TunModeService_object == 0) {
			return;
		}

		JNIEnv* env = nullptr;
		int status = get_jni_env(&env);

		if (status == 2)
		{
			return;
		}

		jclass TunModeService_class = env->FindClass("git/gxosty/tunmode/interceptor/services/TunModeService");
		jmethodID TunModeService_tunnelClosed_methodID = env->GetMethodID(
			TunModeService_class,
			"tunnelClosed",
			"()V"
		);

		env->CallVoidMethod(params::TunModeService_object, TunModeService_tunnelClosed_methodID);

		if (status == 1) {
			params::jvm->DetachCurrentThread();
		}
	}

	void open_tunnel()
	{
		params::stop_flag.store(false);
		params::tunnel_promise = std::promise<void>();

		std::future tunnel_future = params::tunnel_promise.get_future();

		_run_loops();

		LOGI_("----- [Tunnel opened] -----");
		tunnel_future.wait();

		_cleanup();
		_tunnel_closed();
		LOGI_("----- [Tunnel closed] -----");
	}

	void close_tunnel()
	{
		params::stop_flag.store(true);
	}
}