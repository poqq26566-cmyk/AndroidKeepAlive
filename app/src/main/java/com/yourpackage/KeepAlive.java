package com.yourpackage.keepalive;

import android.app.Application;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import androidx.core.app.NotificationCompat;

public class KeepAlive {
    static {
        System.loadLibrary("keepalive");
    }

    public native void startDaemon();
    public native void antiFreeze();

    public static void start(Context context) {
        KeepAlive ka = new KeepAlive();
        ka.startDaemon();
        ka.antiFreeze();
    }
}

// 前台服务
class ForegroundService extends Service {
    @Override
    public void onCreate() {
        super.onCreate();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                "keepalive", "保活", NotificationManager.IMPORTANCE_LOW
            );
            NotificationManager nm = getSystemService(NotificationManager.class);
            nm.createNotificationChannel(channel);
        }
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, "keepalive")
            .setContentTitle("保活中")
            .setContentText("正在运行...")
            .setSmallIcon(android.R.drawable.ic_menu_save);
        startForeground(1001, builder.build());
    }

    @Override
    public IBinder onBind(Intent intent) { return null; }
}

// 开机自启
class BootReceiver extends android.content.BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) {
            Intent i = new Intent(context, MainActivity.class);
            i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(i);
        }
    }
}

// Application
class MyApplication extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(new Intent(this, ForegroundService.class));
        }
        KeepAlive.start(this);
    }
}
