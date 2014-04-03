/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifndef _ANDROID
#ifndef _WINRT
#define RESOURCE_PATH "../media/"
#else
#define RESOURCE_PATH "media/"
#endif
#else
#define RESOURCE_PATH "./"
#endif

#include <april/april.h>
#include <april/main.h>
#include <april/Platform.h>
#include <april/RenderSystem.h>
#include <april/UpdateDelegate.h>
#include <april/Window.h>

april::Texture* texture = NULL;
april::TexturedVertex v[4];

#if !defined(_ANDROID) && !defined(_IOS) && !defined(_WINP8)
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grect drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif

class UpdateDelegate : public april::UpdateDelegate
{
public:
	UpdateDelegate() : april::UpdateDelegate(), angle(0.0f)
	{
	}

	bool onUpdate(float timeSinceLastFrame)
	{
		this->angle += timeSinceLastFrame * 90.0f;
		april::rendersys->clear();
		april::rendersys->setPerspective(60.0f, 1.0f / drawRect.getAspect(), 1.0f, 1000.0f);
		april::rendersys->lookAt(gvec3(2.0f, 2.0f, -5.0f), gvec3(0.0f, 0.0f, 0.0f), gvec3(0.0f, 1.0f, 0.0f));
		april::rendersys->rotate(this->angle, 0.0f, 1.0f, 0.0f);
		april::rendersys->setTexture(texture);
		april::rendersys->render(april::RO_TRIANGLE_STRIP, v, 4);
		return true;
	}

protected:
	float angle;

};

static UpdateDelegate* updateDelegate = NULL;

void april_init(const harray<hstr>& args)
{
#ifdef __APPLE__
	// On MacOSX, the current working directory is not set by
	// the Finder, since you are expected to use Core Foundation
	// or ObjC APIs to find files. 
	// So, when porting you probably want to set the current working
	// directory to something sane (e.g. .../Resources/ in the app
	// bundle).
	// In this case, we set it to parent of the .app bundle.
	{	// curly braces in order to localize variables 

		CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
		CFStringRef path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
		// let's hope chdir() will be happy with utf8 encoding
		const char* cpath = CFStringGetCStringPtr(path, kCFStringEncodingUTF8);
		char* cpath_alloc = NULL;
		if (cpath == NULL)
		{
			// CFStringGetCStringPtr is allowed to return NULL. bummer.
			// we need to use CFStringGetCString instead.
			cpath_alloc = (char*)malloc(CFStringGetLength(path) + 1);
			CFStringGetCString(path, cpath_alloc, CFStringGetLength(path) + 1, kCFStringEncodingUTF8);
		}
		else
		{
			// even though it didn't return NULL, we still want to slice off bundle name.
			cpath_alloc = (char*)malloc(CFStringGetLength(path) + 1);
			strcpy(cpath_alloc, cpath);
		}
		// just in case / is appended to .app path for some reason
		if (cpath_alloc[CFStringGetLength(path) - 1] == '/')
		{
			cpath_alloc[CFStringGetLength(path) - 1] = 0;
		}
		// replace pre-.app / with a null character, thus
		// cutting off .app's name and getting parent of .app.
		strrchr(cpath_alloc, '/')[0] = 0;
		// change current dir using posix api
		chdir(cpath_alloc);
		free(cpath_alloc); // even if null, still ok
		CFRelease(path);
		CFRelease(url);
	}
#endif
	updateDelegate = new UpdateDelegate();
#if defined(_ANDROID) || defined(_IOS) || defined(_WINRT)
	drawRect.setSize(april::getSystemInfo().displayResolution);
#endif
	april::init(april::RS_DEFAULT, april::WS_DEFAULT);
	april::createRenderSystem();
	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "APRIL: Simple 3D");
#ifdef _WINRT
	april::window->setParam("cursor_mappings", "101 " RESOURCE_PATH "cursor\n102 " RESOURCE_PATH "simple");
#endif
	april::window->setUpdateDelegate(updateDelegate);
	april::window->setCursorFilename(RESOURCE_PATH "cursor");
	texture = april::rendersys->createTextureFromResource(RESOURCE_PATH "texture");
	v[0].x = -1.0f;	v[0].y = 1.0f;	v[0].z = 0.0f;	v[0].u = 0.0f;	v[0].v = 0.0f;
	v[1].x = 1.0f;	v[1].y = 1.0f;	v[1].z = 0.0f;	v[1].u = 1.0f;	v[1].v = 0.0f;
	v[2].x = -1.0f;	v[2].y = -1.0f;	v[2].z = 0.0f;	v[2].u = 0.0f;	v[2].v = 1.0f;
	v[3].x = 1.0f;	v[3].y = -1.0f;	v[3].z = 0.0f;	v[3].u = 1.0f;	v[3].v = 1.0f;
}

void april_destroy()
{
	delete texture;
	april::destroy();
	delete updateDelegate;
	updateDelegate = NULL;
}
