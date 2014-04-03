// the name of the package, make sure it matches "<manifest package" in AndroidManifest.xml
package com.example.aprilExample;

// the name of the class, make sure it matches "<activity android:name" in AndroidManifest.xml
public class ExampleActivity extends net.sourceforge.april.AprilActivity
{
	static
	{
		// this is the name of your app's .so file without the prefix "lib" and the suffix ".so"
		System.loadLibrary("april_example");
	}
	
}
