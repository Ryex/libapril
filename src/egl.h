/// @file
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
/// Defines EGL utility APi.

#ifdef _EGL
#ifndef APRIL_EGL_H
#define APRIL_EGL_H

#include <EGL/egl.h>
#include <gtypes/Vector3.h>

#include "aprilExport.h"

namespace april
{
	class EglData
	{
	public:
		EGLNativeWindowType hWnd;
		EGLDisplay display;
		EGLConfig config;
		EGLSurface surface;
		EGLContext context;
		EGLint pi32ConfigAttribs[128];

		EglData();
		~EglData();

		bool create();
		bool destroy();
		void swapBuffers();

	};

	extern EglData* egl;

}
#endif
#endif