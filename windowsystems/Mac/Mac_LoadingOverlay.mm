/// @file
/// @author  Kresimir Spes
/// @version 3.3
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#import <Cocoa/Cocoa.h>
#import "Mac_LoadingOverlay.h"
#include "Mac_Window.h"

NSWindow* mOverlayWindow = nil;
NSImageView* mImageView = nil;

static void updateLoadingOverlaySize(NSWindow* parent, bool check)
{
	NSRect windowFrame = parent.frame;
	NSRect frame = [parent.contentView bounds];
	
	if (!check || !NSEqualSizes(frame.size, mOverlayWindow.frame.size))
	{
		frame.origin = windowFrame.origin;

		NSSize imgSize = [mImageView image].size;
		NSRect imgFrame = frame;
		if ((float) imgSize.width / imgSize.height > (float) frame.size.width / frame.size.height)
		{
			float w = frame.size.width;
			imgFrame.size.width = frame.size.height * imgSize.width / imgSize.height;
			imgFrame.origin.y = 0;
			imgFrame.origin.x = -(imgFrame.size.width - w) / 2;
		}
		else
		{
			imgFrame.origin.x = imgFrame.origin.y = 0;
		}
		[mImageView setFrame:imgFrame];
		[mOverlayWindow setFrame:frame display:YES];
	}
}

void createLoadingOverlay(NSWindow* parent)
{
	NSString* path = [[NSBundle mainBundle] pathForResource:@"Default" ofType:@"png"];
	bool found = [[NSFileManager defaultManager] fileExistsAtPath:path];
	
	if (!found)
	{
		path = [[NSBundle mainBundle] pathForResource:@"Default-Mac" ofType:@"png"];
		found = [[NSFileManager defaultManager] fileExistsAtPath:path];
	}
	
	if (found)
	{
		NSRect windowFrame = parent.frame;
		NSRect frame = [parent.contentView bounds];

		frame.origin = windowFrame.origin;
		mOverlayWindow = [[NSWindow alloc] initWithContentRect:frame styleMask:NSBorderlessWindowMask backing: NSBackingStoreBuffered defer:false];
		[mOverlayWindow setBackgroundColor:[NSColor blackColor]];
		[mOverlayWindow setOpaque:YES];

		NSImage* image = [[NSImage alloc] initWithContentsOfFile:path];
		mImageView = [[NSImageView alloc] init];
		[mImageView setImage:image];

		updateLoadingOverlaySize(parent, 0);

		[mImageView setImageScaling:NSImageScaleAxesIndependently];

		[mOverlayWindow.contentView addSubview: mImageView];
		[parent addChildWindow:mOverlayWindow ordered:NSWindowAbove];
		[mOverlayWindow makeKeyWindow];
	}
}

void reattachLoadingOverlay()
{
	if (mOverlayWindow != nil)
	{
		NSWindow* wnd = mOverlayWindow.parentWindow;
		[wnd removeChildWindow:mOverlayWindow];
		[wnd addChildWindow:mOverlayWindow ordered:NSWindowAbove];
	}
}

void updateLoadingOverlay(float k)
{
	static float alpha = 505;
	if (alpha == 505)
	{
		alpha = aprilWindow->getParam("splashscreen_fadeout") == "0" ? -1 : 1.5f;
	}
	if (mOverlayWindow)
	{
		if (aprilWindow->getParam("fasthide_loading_overlay") == "1")
			alpha -= k * 3; // If you don't want to wait for the loading overlay every time, set this param. eg on debug mode
		else
			alpha -= k;

		if (aprilWindow->getParam("retain_loading_overlay") == "1") alpha = 1.5f;

		if (alpha < 0)
		{
			[mImageView removeFromSuperview];
			[mImageView release];
			mImageView = nil;
			[mOverlayWindow.parentWindow removeChildWindow:mOverlayWindow];
			[mOverlayWindow release];
			mOverlayWindow = nil;
		}
		else
		{
			mOverlayWindow.alphaValue = alpha;
			updateLoadingOverlaySize([mOverlayWindow parentWindow], 1);
		}
	}
}
