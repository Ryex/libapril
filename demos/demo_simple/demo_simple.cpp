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
#include <april/MouseDelegate.h>
#include <april/Platform.h>
#include <april/RenderSystem.h>
#include <april/UpdateDelegate.h>
#include <april/Window.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#define LOG_TAG "demo_simple"

april::Texture* texture = NULL;
april::Texture* manualTexture = NULL;
april::TexturedVertex dv[4];
#if !defined(_ANDROID) && !defined(_IOS) && !defined(_WINP8)
grect drawRect(0.0f, 0.0f, 800.0f, 600.0f);
#else
grect drawRect(0.0f, 0.0f, 480.0f, 320.0f);
#endif
gvec2 offset = drawRect.getSize() * 0.5f;
grect textureRect;
grect src(0.0f, 0.0f, 1.0f, 1.0f);
bool mousePressed = false;

class UpdateDelegate : public april::UpdateDelegate
{
	bool onUpdate(float timeSinceLastFrame)
	{
		april::rendersys->clear();
		april::rendersys->setOrthoProjection(drawRect);
		april::rendersys->drawFilledRect(drawRect, april::Color(96, 96, 96));
		manualTexture->fillRect(hrand(manualTexture->getWidth()), hrand(manualTexture->getHeight()), hrand(1, 9), hrand(1, 9), april::Color(hrand(255), hrand(255), hrand(255)));
		april::rendersys->setTexture(manualTexture);
		april::rendersys->render(april::RO_TRIANGLE_STRIP, dv, 4);
		april::rendersys->setTexture(texture);
		april::rendersys->drawTexturedRect(textureRect + offset, src);
		april::rendersys->drawFilledRect(grect(0.0f, drawRect.h - 75.0f, 100.0f, 75.0f), april::Color::Yellow);
		april::rendersys->drawFilledRect(grect(10.0f, drawRect.h - 65.0f, 80.0f, 55.0f), april::Color::Red);
		return true;
	}

};

class MouseDelegate : public april::MouseDelegate
{
	void onMouseDown(april::Key key)
	{
		offset = april::window->getCursorPosition();
		hlog::writef(LOG_TAG, "- DOWN x: %4.0f y: %4.0f button: %d", offset.x, offset.y, key);
		mousePressed = true;
	}

	void onMouseUp(april::Key key)
	{
		gvec2 cursor = april::window->getCursorPosition();
		hlog::writef(LOG_TAG, "- UP   x: %4.0f y: %4.0f button: %d", cursor.x, cursor.y, key);
		mousePressed = false;
	}

	void onMouseMove()
	{
		gvec2 cursor = april::window->getCursorPosition();
		hlog::writef(LOG_TAG, "- MOVE x: %4.0f y: %4.0f", cursor.x, cursor.y);
		if (mousePressed)
		{
			offset = cursor;
		}
	}

	void onMouseCancel(april::Key key)
	{
		hlog::writef(LOG_TAG, "- CANCEL button: %d", key);
	}

};

static UpdateDelegate* updateDelegate = NULL;
static MouseDelegate* mouseDelegate = NULL;

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
	mouseDelegate = new MouseDelegate();
#if defined(_ANDROID) || defined(_IOS) || defined(_WINRT)
	drawRect.setSize(april::getSystemInfo().displayResolution);
#endif
	srand(get_system_time());
	dv[0].x = 0.0f;			dv[0].y = 0.0f;			dv[0].z = 0.0f;	dv[0].u = 0.0f;	dv[0].v = 0.0f;
	dv[1].x = drawRect.w;	dv[1].y = 0.0f;			dv[1].z = 0.0f;	dv[1].u = 1.0f;	dv[1].v = 0.0f;
	dv[2].x = 0.0f;			dv[2].y = drawRect.h;	dv[2].z = 0.0f;	dv[2].u = 0.0f;	dv[2].v = 1.0f;
	dv[3].x = drawRect.w;	dv[3].y = drawRect.h;	dv[3].z = 0.0f;	dv[3].u = 1.0f;	dv[3].v = 1.0f;
	april::init(april::RS_DEFAULT, april::WS_DEFAULT);
	april::createRenderSystem();
	april::createWindow((int)drawRect.w, (int)drawRect.h, false, "APRIL: Simple Demo");
#ifdef _WINRT
	april::window->setParam("cursor_mappings", "101 " RESOURCE_PATH "cursor\n102 " RESOURCE_PATH "simple");
#endif
	april::window->setUpdateDelegate(updateDelegate);
	april::window->setMouseDelegate(mouseDelegate);
	april::window->setCursorFilename(RESOURCE_PATH "cursor");
	texture = april::rendersys->createTextureFromResource(RESOURCE_PATH "jpt_final", april::Texture::TYPE_MANAGED);
	textureRect.setSize(texture->getWidth() * 0.5f, texture->getHeight() * 0.5f);
	textureRect.x = -textureRect.w / 2;
	textureRect.y = -textureRect.h / 2;
	// demonstrating some of the image manipulation methods
	manualTexture = april::rendersys->createTexture((int)drawRect.w, (int)drawRect.h, april::Color::Clear, april::Image::FORMAT_RGBA, april::Texture::TYPE_MANAGED);
	manualTexture->write(0, 0, texture->getWidth(), texture->getHeight(), 0, 0, texture);
	manualTexture->invert(0, 0, 256, 128);
	manualTexture->saturate(0, 128, 128, 128, 0.0f);
	manualTexture->rotateHue(128, 0, 128, 128, 180.0f);
	manualTexture->blit(0, 0, texture->getWidth(), texture->getHeight(), 128, 128, texture, 96);
	manualTexture->blitStretch(texture->getWidth() / 2, 0, texture->getWidth() / 2, texture->getHeight(), 64, 128, 700, 200, texture, 224);
}

void april_destroy()
{
	delete texture;
	delete manualTexture;
	april::destroy();
	delete updateDelegate;
	updateDelegate = NULL;
	delete mouseDelegate;
	mouseDelegate = NULL;
}
