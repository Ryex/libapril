package com.example.april.android.demoSimple;

public class DemoSimple extends net.sourceforge.april.android.Activity
{
	static
	{
		System.loadLibrary("demo_simple");
	}
	
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		this.forceArchivePath(net.sourceforge.april.android.NativeInterface.ApkPath); // forces APK as archive file
	}
	
}
