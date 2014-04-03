/// @file
/// @author  Kresimir Spes
/// @version 3.3
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <Cocoa/Cocoa.h>
#include <hltypes/hlog.h>
#include "april.h"
#include "Mac_Window.h"
#import <OpenGL/gl.h>
#import "Mac_OpenGLView.h"

@implementation AprilMacOpenGLView

- (id) initWithFrame:(NSRect)frameRect
{
	mUseBlankCursor = false;
	mStartedDrawing = false;
	
	NSImage* image = [[NSImage alloc] initWithSize: NSMakeSize(8, 8)];
	
	NSBitmapImageRep* bmp = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes: NULL pixelsWide: 1 pixelsHigh: 1 bitsPerSample: 8 samplesPerPixel: 4 hasAlpha: YES isPlanar: NO colorSpaceName: NSDeviceRGBColorSpace bytesPerRow: 0 bitsPerPixel: 0];
	[image addRepresentation: bmp];
	[image lockFocus];
	
	[[[NSColor blackColor] colorWithAlphaComponent:0] set];
	NSRectFill(NSMakeRect(0, 0, 1, 1));
	
	[image unlockFocus];

	mBlankCursor = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(0, 0)];
	[bmp release];
	[image release];

	// set up pixel format
	int n = 0;
	NSOpenGLPixelFormatAttribute a[64] = {0};
	a[n++] = NSOpenGLPFANoRecovery;
	a[n++] = NSOpenGLPFADoubleBuffer;
	a[n++] = NSOpenGLPFADepthSize; a[n++] = (NSOpenGLPixelFormatAttribute) 16;
	if (isLionOrNewer())
	{
		a[n++] = kCGLPFAOpenGLProfile; a[n++] = kCGLOGLPVersion_Legacy;
	}
	a[n++] = 0;

    NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:a];

    if (!pf) hlog::error(april::logTag, "Unable to create requested OpenGL pixel format");

    if (self = [super initWithFrame:frameRect pixelFormat:[pf autorelease]])
	{
		[self initGL];
	}
	
	return self;
}

- (void)updateGLViewport
{
	glViewport(0, 0, aprilWindow->getWidth(), aprilWindow->getHeight());
}

- (void)drawRect:(NSRect)dirtyRect
{
	if (aprilWindow->ignoreUpdate)
	{
		mStartedDrawing = false;
		return;
	}
	NSOpenGLContext* context = [self openGLContext];
	[context makeCurrentContext];
	if (april::window != NULL)
	{
		aprilWindow->updateOneFrame();
		if (april::rendersys != NULL)
		{
			[self presentFrame];
		}
	}
	mStartedDrawing = false;
}

- (void) initGL
{
	NSOpenGLContext* context = [self openGLContext];
	[context makeCurrentContext];
	// Synchronize buffer swaps with vertical refresh rate
	GLint swapInt = 1;
	[context setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

- (void) presentFrame
{
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] flushBuffer];
}

- (void)resetCursorRects
{
	if (mUseBlankCursor)
	{
		[self addCursorRect:[self bounds] cursor:mBlankCursor];
	}
}

// Apple doesn't forward rightmouse events on MacOS 10.6 and earlier for some reason, so we override the behaviour here
- (void)rightMouseDown:(NSEvent*) event
{
	[[self nextResponder] rightMouseDown:event];
}

- (void)rightMouseUp:(NSEvent*) event
{
	[[self nextResponder] rightMouseUp:event];
}


- (void) setDefaultCursor
{
	mUseBlankCursor = false;
}

- (void) setBlankCursor
{
	mUseBlankCursor = true;
}

- (void) dealloc
{
	[mBlankCursor release];
	[super dealloc];
}

@end
