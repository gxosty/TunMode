package git.gxosty.tunmode;

import android.app.Application;
import android.app.NotificationChannel;
import android.app.NotificationManager;

import android.os.Build;

import android.content.Context;

public class TunModeApp extends Application {
	public static final String VPN_SERVICE_NOTIFICATION_CHANNEL = "tun_mode_vpn_service_notifi_channel";
	public static final Integer VPN_SERVICE_NOTIFICATION_ID = 1337;

	@Override
	public void onCreate() {
		super.onCreate();

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
			NotificationManager notifManager = ((NotificationManager)this.getSystemService(Context.NOTIFICATION_SERVICE));
			NotificationChannel notifChannel = notifManager.getNotificationChannel(VPN_SERVICE_NOTIFICATION_CHANNEL);

			if (notifChannel == null) {
				notifChannel = new NotificationChannel(
					VPN_SERVICE_NOTIFICATION_CHANNEL,
					"TunMode Enabled",
					NotificationManager.IMPORTANCE_MIN
				);

				notifChannel.setDescription("Shown when TunMode is enabled");
				notifManager.createNotificationChannel(notifChannel);
			}
		}
	}
}