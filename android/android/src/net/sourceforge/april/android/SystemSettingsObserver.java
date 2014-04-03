package net.sourceforge.april.android;

// version 3.3

import android.content.pm.ActivityInfo;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.provider.Settings.System;

public class SystemSettingsObserver extends ContentObserver
{
	public SystemSettingsObserver()
	{
		super(new Handler());
	}
	
	@Override
	public boolean deliverSelfNotifications()
	{
		return true;
	}

	@Override
	public void onChange(boolean selfChange, Uri uri)
	{
		// check if auto-rotation is supported
		if (Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.GINGERBREAD)
		{
			return;
		}
		// enable auto-rotation
		if (System.getInt(NativeInterface.Activity.getContentResolver(), System.ACCELEROMETER_ROTATION, 0) != 0)
		{
			if (NativeInterface.Activity.getRequestedOrientation() == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE)
			{
				NativeInterface.Activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
			}
			else if (NativeInterface.Activity.getRequestedOrientation() == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT)
			{
				NativeInterface.Activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT);
			}
		}
		else
		{
			if (NativeInterface.Activity.getRequestedOrientation() == ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE)
			{
				NativeInterface.Activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
			}
			else if (NativeInterface.Activity.getRequestedOrientation() == ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT)
			{
				NativeInterface.Activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
			}
		}
	}

	@Override
	public void onChange(boolean selfChange)
	{
		this.onChange(selfChange, null);
	}
	
};
