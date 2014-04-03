package com.example.april.android.demo3d;

public class Demo3d extends net.sourceforge.april.android.Activity
{
	static
	{
		System.loadLibrary("demo_3d");
	}
	
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		this.forceArchivePath(net.sourceforge.april.android.NativeInterface.ApkPath); // forces APK as archive file
	}
	
}
