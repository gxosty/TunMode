#include <tunmode/tunmode.hpp>
#include <misc/logger.hpp>

#include <atomic>
#include <future>
#include <string>
#include <vector>
#include <thread>

#include <poll.h>
#include <unistd.h>

#define TAG "TunMode"

namespace tunmode
{
	namespace params
	{
		JavaVM* jvm;
		int tun_fd;
		jobject TunModeService_object;

		std::atomic<bool> stop_flag;
		std::promise<void> tunnel_promise;
		std::atomic<int> thread_count;
	}

	void set_jvm(JavaVM* jvm)
	{
		params::jvm = jvm;
	}

	void initialize(JNIEnv* env, jobject TunModeService_object)
	{
		params::tun_fd = 0;
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

		struct pollfd socket_fds[1];

		socket_fds[0].fd = params::tun_fd;
		socket_fds[0].events = POLLIN;
		socket_fds[0].revents = 0;

		while (!params::stop_flag.load())
		{
			int ret = poll(socket_fds, 1, 2000);

			if (ret == -1)
			{
				LOGE(TAG, "Poll Error");
			}
			else if (ret == 0)
			{
				continue;    // Timeout reached
			}
			else
			{
				if (socket_fds[0].revents & POLLIN)
				{

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
		close(params::tun_fd);
		params::tun_fd = 0;

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

		tunnel_future.wait();

		_cleanup();
		_tunnel_closed();
	}

	void close_tunnel()
	{
		params::stop_flag.store(true);
	}
}