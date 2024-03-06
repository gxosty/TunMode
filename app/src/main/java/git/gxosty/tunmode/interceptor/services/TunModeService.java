package git.gxosty.tunmode.interceptor.services;

import android.os.Build;
import android.os.ParcelFileDescriptor;

import android.net.VpnService;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.LinkProperties;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ServiceInfo;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;

import androidx.appcompat.app.AppCompatActivity;

import androidx.core.app.NotificationCompat;
import androidx.core.app.TaskStackBuilder;

import java.lang.Thread;
import java.lang.System;

import git.gxosty.tunmode.R;
import git.gxosty.tunmode.TunModeApp;

public class TunModeService extends VpnService {
	/**
	 * TUN address
	 *
	 * Address on which to route device traffic
	 **/
	public static final String tunAddress = "10.0.0.1";

	/**
	 * DNS Address
	 *
	 * Set dns resolver address (e.g. Google Public DNS 8.8.8.8)
	 *
	 * You may also set local imaginary fake IP address (e.g. 10.0.0.2)
	 * to route dns queries over TUN socket. Note that you will handle
	 * query packets yourself!
	 *
	 * Set to `null` if you want to use system dns resolver
	 **/
	public static final String dnsAddress = "8.8.8.8";

	public static final String INTENT_EXTRA_OPERATION = "TunModeService_Operation";
	public static final String NOTIFICATION_CHANNEL = "tun_mode_vpn_service_nc";

	private static State state = TunModeService.State.DISCONNECTED;
	private static EventListener eventListener = null;
	private static AppCompatActivity activity = null;

	private Notification notif;
	private ParcelFileDescriptor tunnel;

	static {
		System.loadLibrary("tunmode");
	}

	public enum State {
		CONNECTING,
		CONNECTED,
		DISCONNECTING,
		DISCONNECTED
	};

	public enum Event {
		INITIALIZED,
		COULDNT_INITIALIZE,
		CONNECTING,
		CONNECTED,
		DISCONNECTING,
		DISCONNECTED,
		NETWORK_ERROR
	};

	public enum Operation {
		INITIALIZE,
		CONNECT,
		DISCONNECT
	};

	@Override
	public void onCreate() {
		super.onCreate();
		TunModeService.setupNative(this);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		if (intent == null) {
			return VpnService.START_NOT_STICKY;
		}

		Operation operation = (Operation)intent.getSerializableExtra(INTENT_EXTRA_OPERATION);

		switch (operation) {
		case INITIALIZE:
			break;

		case CONNECT:
			if (this.activity != null) {
				Intent vpnIntent = this.prepare(this.activity);

				if (vpnIntent != null) {
					activity.startActivityForResult(vpnIntent, 1337);
					return VpnService.START_NOT_STICKY;
				}
			}

			NotificationCompat.Builder builder = new NotificationCompat.Builder(this, TunModeApp.VPN_SERVICE_NOTIFICATION_CHANNEL);

			builder.setSmallIcon(R.drawable.ic_launcher_foreground)
				.setContentTitle("TunMode")
				.setContentText("Connected")
				.setOngoing(true);

			this.notif = builder.build();

			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
				this.startForeground(TunModeApp.VPN_SERVICE_NOTIFICATION_ID, this.notif, ServiceInfo.FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE);
			} else {
				this.startForeground(TunModeApp.VPN_SERVICE_NOTIFICATION_ID, this.notif);
			}

			this.connect();
			break;

		case DISCONNECT:
			this.disconnect();
			break;
		}

		return VpnService.START_NOT_STICKY;
	}

	private void tunnelClosed() {
		this.tunnel = null;
		TunModeService.setState(State.DISCONNECTED);
		this.sendEvent(Event.DISCONNECTED);

		if (this.eventListener == null) {
			this.stopSelf();
		} else {
			this.stopForeground(true);
		}

		if (this.notif != null) {
			NotificationManager notifManager = (NotificationManager)this.getSystemService(Context.NOTIFICATION_SERVICE);
			notifManager.cancel(TunModeApp.VPN_SERVICE_NOTIFICATION_ID);
			this.notif = null;
		}
	}

	private void connect() {
		if (tunnel != null) {
			return;
		}

		ConnectivityManager connectivityManager = this.getSystemService(ConnectivityManager.class);
		Network currentNetwork = connectivityManager.getActiveNetwork();

		if (currentNetwork == null) {
			this.sendEvent(Event.NETWORK_ERROR);
			return;
		}

		NetworkCapabilities networkCapabilities = connectivityManager.getNetworkCapabilities(currentNetwork);
		LinkProperties linkProperties = connectivityManager.getLinkProperties(currentNetwork);

		/* Check if network interface has access to internet */
		// if (!networkCapabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
		// 	|| !networkCapabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED)) {
		// 	this.sendEvent(Event.NETWORK_ERROR);
		// 	return;
		// }

		String networkInterface = linkProperties.getInterfaceName();

		if (networkInterface == null) {
			this.sendEvent(Event.DISCONNECTED);
			return;
		}

		TunModeService.setState(State.CONNECTING);
		this.sendEvent(Event.CONNECTING);

		new Thread(() -> {
			VpnService.Builder builder = new VpnService.Builder()
				.addAddress(TunModeService.tunAddress, 32)
				.addRoute("0.0.0.0", 0)
				.setMtu(8192);

			if (TunModeService.dnsAddress != null) {
				// route dns queries as well
				builder.addDnsServer(dnsAddress);
			}

			this.tunnel = builder.establish();

			if (this.tunnel != null) {
				TunModeService.setState(State.CONNECTED);
				this.sendEvent(Event.CONNECTED);
				TunModeService.tunnelOpenNative(this.tunnel.detachFd(), networkInterface, dnsAddress);
			} else {
				TunModeService.setState(State.DISCONNECTED);
				this.tunnelClosed();
			}
		}).start();
	}

	private void disconnect() {
		if (this.tunnel != null) {
			TunModeService.setState(State.DISCONNECTING);
			this.sendEvent(Event.DISCONNECTING);
			TunModeService.tunnelCloseNative();
		}
	}

	public static State getState() {
		return TunModeService.state;
	}

	private static void setState(State state) {
		TunModeService.state = state;
	}

	public interface EventListener {
		public void onEvent(Event event);
	}

	public static void setEventListener(EventListener eventListener) {
		TunModeService.eventListener = eventListener;
	}

	public static void setActivity(AppCompatActivity activity) {
		TunModeService.activity = activity;
	}

	private static void sendEvent(Event event) {
		if (TunModeService.eventListener != null) {
			TunModeService.eventListener.onEvent(event);
		}
	}

	private static native void setupNative(Object service);
	private static native void tunnelOpenNative(int fd, String net_iface, String dns_address);
	private static native void tunnelCloseNative();
}