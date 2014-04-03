package net.sourceforge.april.android;

// version 3.3

import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.ResultReceiver;
import android.view.inputmethod.InputMethodManager;
import android.view.View;
import android.util.DisplayMetrics;

import net.sourceforge.april.android.DialogFragment;

import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.lang.Math;
import java.lang.Runtime;
import java.util.Locale;

public class NativeInterface
{
	public static android.app.Activity Activity = null;
	public static Activity AprilActivity = null;
	public static boolean Running = false;
	public static boolean KeyboardVisible = false;
	public static String ArchivePath = "";
	public static String DataPath = ".";
	public static String PackageName = "";
	public static String VersionCode = "0";
	public static String ApkPath = "";
	
	private static boolean htcKeyboardHack = false;
	private static class KeyboardResultReceiver extends ResultReceiver
	{
		public KeyboardResultReceiver()
		{
			super(new Handler());
		}
		
		@Override
		protected void onReceiveResult(int resultCode, Bundle resultData)
		{
			boolean keyboardVisible = true;
			if (resultCode == InputMethodManager.RESULT_UNCHANGED_HIDDEN ||
				resultCode == InputMethodManager.RESULT_HIDDEN)
			{
				keyboardVisible = false;
			}
			if (keyboardVisible && NativeInterface.htcKeyboardHack)
			{
				NativeInterface.htcKeyboardHack = false;
				InputMethodManager inputMethodManager = NativeInterface._getInputMethodManager();
				View view = NativeInterface.AprilActivity.getView();
				inputMethodManager.hideSoftInputFromWindow(view.getWindowToken(), 0, NativeInterface.keyboardResultReceiver);
				inputMethodManager.showSoftInput(view, 0, NativeInterface.keyboardResultReceiver);
			}
		}
		
	};
	private static KeyboardResultReceiver keyboardResultReceiver = new KeyboardResultReceiver();
	
	public static native void setVariables(String dataPath, String forcedArchivePath);
	public static native void init(String[] args);
	public static native boolean render();
	public static native void destroy();
	public static native void onKeyDown(int keyCode, int charCode);
	public static native void onKeyUp(int keyCode);
	public static native void onTouch(int type, float x, float y, int index);
	public static native void onButtonDown(int buttonCode);
	public static native void onButtonUp(int buttonCode);
	public static native void onWindowFocusChanged(boolean focused);
	public static native void onVirtualKeyboardChanged(boolean visible, float heightRation);
	public static native void onLowMemory();
	public static native void onSurfaceCreated();
	
	public static native void activityOnCreate();
	public static native void activityOnStart();
	public static native void activityOnResume();
	public static native void activityOnPause();
	public static native void activityOnStop();
	public static native void activityOnDestroy();
	public static native void activityOnRestart();
	
	public static native void onDialogOk();
	public static native void onDialogYes();
	public static native void onDialogNo();
	public static native void onDialogCancel();
	
	public static String getUserDataPath()
	{
		return NativeInterface.Activity.getFilesDir().getAbsolutePath();
	}
	
	public static Object getDisplayResolution()
	{
		DisplayMetrics metrics = new DisplayMetrics();
		NativeInterface.Activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
		int width = metrics.widthPixels;
		int height = metrics.heightPixels;
		if (height > width)
		{
			height = metrics.widthPixels;
			width = metrics.heightPixels;
		}
		// fixes problem with bottom 20 pixels being covered by Kindle Fire's menu
		if (Build.MANUFACTURER.equals("Amazon") && Build.MODEL.equals("Kindle Fire"))
		{
			height -= 20;
		}
		int[] result = {width, height};
		return result;
	}
	
	public static int getDisplayDpi()
	{
		// hardcoded exceptions for known devices that return wrong DPI
		if (Build.MANUFACTURER.equals("HTC") && Build.MODEL.equals("HTC One X"))
		{
			return 312;
		}
		DisplayMetrics metrics = new DisplayMetrics();
		NativeInterface.Activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
		return (int)Math.sqrt((metrics.xdpi * metrics.xdpi + metrics.ydpi * metrics.ydpi) / 2.0);
	}
	
	public static String getOsVersion()
	{
		return Build.VERSION.RELEASE;
	}
	
	public static String getLocale()
	{
		Locale locale = Locale.getDefault();
		String result = locale.getLanguage();
		String country = locale.getCountry();
		if (result.equals("pt") && country.equals("PT")) // Java is stupid and needs "equals" instead of "=="
		{
			result += "-" + country;
		}
		return result;
	}
	
	public static void showVirtualKeyboard()
	{
		NativeInterface.Activity.runOnUiThread(new Runnable()
		{
			public void run()
			{
				View view = NativeInterface.AprilActivity.getView();
				NativeInterface._getInputMethodManager().showSoftInput(view, 0, NativeInterface.keyboardResultReceiver);
			}
		});
	}
	
	public static void hideVirtualKeyboard()
	{
		NativeInterface.Activity.runOnUiThread(new Runnable()
		{
			public void run()
			{
				View view = NativeInterface.AprilActivity.getView();
				NativeInterface._getInputMethodManager().hideSoftInputFromWindow(view.getWindowToken(), 0,
					NativeInterface.keyboardResultReceiver);
			}
		});
	}
	
	public static void updateKeyboard()
	{
		// TODO - detect broken versions of com.htc.android.htcime
		if (Build.BOARD.equals("mecha") ||		// Thunderbolt
			Build.BOARD.equals("marvel") ||		// Wildfire S
			Build.BOARD.equals("marvelc"))		// Wildfire S
		{
			htcKeyboardHack = true;
		}
		else if (Build.VERSION.SDK_INT < 10 &&
			Build.BOARD.equals("shooteru") ||	// EVO 3D
			Build.BOARD.equals("supersonic"))	// EVO 4G
		{
			htcKeyboardHack = true;
		}
		else if (Build.VERSION.SDK_INT >= 10 &&
			Build.BOARD.equals("inc"))			// Droid Incredible
		{
			htcKeyboardHack = true;
		}
	}
	
	private static InputMethodManager _getInputMethodManager()
	{
		return (InputMethodManager)NativeInterface.Activity.getSystemService(Context.INPUT_METHOD_SERVICE);
	}
	
	public static void showMessageBox(String title, String text, String ok, String yes, String no, String cancel, int iconId)
	{
		DialogFactory.create(title, text, ok, yes, no, cancel, iconId);
	}
	
	public static void swapBuffers()
	{
		NativeInterface.AprilActivity.GlView.swapBuffers();
	}
	
	public static void reset()
	{
		NativeInterface.Activity = null;
		NativeInterface.AprilActivity = null;
		NativeInterface.Running = false;
		NativeInterface.ArchivePath = "";
		NativeInterface.DataPath = ".";
		NativeInterface.PackageName = "";
		NativeInterface.VersionCode = "0";
		NativeInterface.ApkPath = "";
	}
	
}
