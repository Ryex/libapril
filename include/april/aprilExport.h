/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines macros for DLL exports/imports.

#ifndef APRIL_EXPORT_H
#define APRIL_EXPORT_H

	#ifdef _LIB
		#define aprilExport
		#define aprilFnExport
	#else
		#ifdef _WIN32
			#ifdef APRIL_EXPORTS
				#define aprilExport __declspec(dllexport)
				#define aprilFnExport __declspec(dllexport)
			#else
				#define aprilExport __declspec(dllimport)
				#define aprilFnExport __declspec(dllimport)
			#endif
		#else
			#define aprilExport __attribute__ ((visibility("default")))
			#define aprilFnExport __attribute__ ((visibility("default")))
		#endif
	#endif
	#ifndef DEPRECATED_ATTRIBUTE
		#ifdef _MSC_VER
			#define DEPRECATED_ATTRIBUTE __declspec(deprecated("function is deprecated"))
		#else
			#define DEPRECATED_ATTRIBUTE __attribute__((deprecated))
		#endif
	#endif

#endif
