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
	public static final String tunAddress = "192.168.0.127";
	public static final String dnsAddress = "192.168.0.128";

	public static final String INTENT_EXTRA_OPERATION = "TunModeService_Operation";
	public static final String NOTIFICATION_CHANNEL = "tun_mode_vpn_service_nc";

	private static State state;
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
			if (TunModeService.getState() == null) {
				TunModeService.setState(TunModeService.State.DISCONNECTED);
			}

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

		TunModeService.setState(State.CONNECTING);
		this.sendEvent(Event.CONNECTING);

		new Thread(() -> {
			VpnService.Builder builder = new VpnService.Builder()
				.addAddress(TunModeService.tunAddress, 32)
				// .addRoute("0.0.0.0", 0)
				.addRoute("142.251.37.68", 32)
				// .addRoute(dnsAddress, 32)
				// .addDnsServer(dnsAddress) // route all dns queries as well
				.setMtu(8192);

			this.tunnel = builder.establish();

			if (this.tunnel != null) {
				TunModeService.setState(State.CONNECTED);
				this.sendEvent(Event.CONNECTED);
				TunModeService.tunnelOpenNative(this.tunnel.detachFd(), dnsAddress, "wlan0");
			} else {
				TunModeService.setState(State.DISCONNECTED);
				this.sendEvent(Event.DISCONNECTED);
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
	private static native void tunnelOpenNative(int fd, String dns_address, String network_interface);
	private static native void tunnelCloseNative();
}