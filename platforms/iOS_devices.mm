/// @file
/// @author  Kresimir Spes
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "Platform.h"

void getStaticiOSInfo(chstr name, april::SystemInfo& info)
{
	// source: https://github.com/erica/uidevice-extension/blob/master/UIDevice-Hardware.m
	if (name.starts_with("iPad"))
	{
		if (name.starts_with("iPad1"))
		{
			info.name = "iPad1";
			info.ram = 256;
			info.displayDpi = 132;
		}
		else if (name.starts_with("iPad2"))
		{
			info.name = "iPad2";
			info.ram = 512;
			info.cpuCores = 2;
			if (name == "iPad2,5" || name == "iPad2,6" || name == "iPad2,7") // iPad mini
			{
				info.name = "iPad Mini";
				info.displayDpi = 163;
			}
			else
				info.displayDpi = 132;
		}
		else if (name.starts_with("iPad3"))
		{
			info.name = "iPad3";
			info.ram = 1024;
			info.cpuCores = 2;
			info.displayDpi = 264;
		}
		else if (name.starts_with("iPad4"))
		{
			info.name = "iPad4";
			info.ram = 1024;
			info.cpuCores = 2;
			if (name == "iPad4,4" || name == "iPad4,5") // iPad mini 2
			{
				info.name = "iPad Mini 2";
				info.displayDpi = 326;
			}
			else
				info.displayDpi = 264;
		}
		else
		{
			info.name = "iPad?";
			info.ram = 1024;
			info.cpuCores = 2;
			info.displayDpi = 264;
		}
	}
	else if (name.starts_with("iPhone"))
	{
		if (name == "iPhone1,1")
		{
			info.name = "iPhone2G";
			info.ram = 128;
			info.displayDpi = 163;
		}
		else if (name == "iPhone1,2")
		{
			info.name = "iPhone3G";
			info.ram = 128;
			info.displayDpi = 163;
		}
		else if (name == "iPhone2,1")
		{
			info.name = "iPhone3GS";
			info.ram = 256;
			info.displayDpi = 163;
		}
		else if (name.starts_with("iPhone3"))
		{
			info.name = "iPhone4";
			info.ram = 512;
			info.displayDpi = 326;
		}
		else if (name.starts_with("iPhone4"))
		{
			info.name = "iPhone4S";
			info.cpuCores = 2;
			info.ram = 512;
			info.displayDpi = 326;
		}
		else if (name.starts_with("iPhone5"))
		{
			info.name = "iPhone5";
			info.cpuCores = 2;
			info.ram = 1024;
			info.displayDpi = 326;
		}
		else if (name.starts_with("iPhone6"))
		{
			info.name = "iPhone5S";
			info.cpuCores = 2;
			info.ram = 1024;
			info.displayDpi = 326;
		}
		else
		{
			info.name = "iPhone?";
			info.ram = 1024;
			info.displayDpi = 326;
		}
	}
	else if (name.starts_with("iPod"))
	{
		if (name == "iPod1,1")
		{
			info.name = "iPod1";
			info.ram = 128;
			info.displayDpi = 163;
		}
		else if (name == "iPod2,1")
		{
			info.name = "iPod2";
			info.ram = 128;
			info.displayDpi = 163;
		}
		else if (name == "iPod3,1")
		{
			info.name = "iPod3";
			info.ram = 256;
			info.displayDpi = 163;
		}
		else if (name == "iPod4,1")
		{
			info.name = "iPod4";
			info.ram = 256;
			info.displayDpi = 326;
		}
		else
		{
			info.name = "iPod?";
			info.ram = 512;
			info.cpuCores = 2;
			info.displayDpi = 326;
		}
	}
	else if (name.starts_with("x86")) // iPhone Simulator
	{
		int w = info.displayResolution.x, h = info.displayResolution.y;
		if (h < 768) // iPhone
		{
			if (h < 640)
			{
				info.name = "iPhone3GS";
				info.ram = 256;
				info.displayDpi = 163;
			}
			else if (w == 960)
			{
				info.name = "iPhone4";
				info.ram = 512;
				info.displayDpi = 326;
			}
			else
			{
				info.name = "iPhone5";
				info.cpuCores = 2;
				info.ram = 1024;
				info.displayDpi = 326;
			}
		}
		else
		{
			if (h == 768)
			{
				info.name = "iPad2";
				info.ram = 512;
				info.cpuCores = 2;
				info.displayDpi = 132;
			}
			else
			{
				info.name = "iPad3";
				info.ram = 1024;
				info.cpuCores = 2;
				info.displayDpi = 264;
			}
		}
	}
	//else: i386 (iphone simulator) and possible future device types
}
