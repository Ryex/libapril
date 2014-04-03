/// @file
/// @author  Kresimir Spes
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION

#ifndef APRIL_MAC_OPENGL_VIEW_H
#define APRIL_MAC_OPENGL_VIEW_H

#import <AppKit/NSOpenGLView.h>

@interface AprilMacOpenGLView : NSOpenGLView
{
@public
	bool mStartedDrawing;
	bool mUseBlankCursor;
	NSCursor* mBlankCursor;
}
- (void) initGL;
- (void) presentFrame;
- (void) setDefaultCursor;
- (void) setBlankCursor;
- (void) updateGLViewport;

@end

#endif
