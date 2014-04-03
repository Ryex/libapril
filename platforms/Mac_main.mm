/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#import <Cocoa/Cocoa.h>

#include "RenderSystem.h"
#include "Window.h"
#include "main_base.h"
#ifdef _COCOA_WINDOW
#import "Mac_Window.h"
#import "Mac_AppDelegate.h"
#endif

int gArgc = 0;
char** gArgv;
void (*gAprilInit)(const harray<hstr>&);
void (*gAprilDestroy)();
static BOOL gFinderLaunch = NO;

int gAprilShouldInvokeQuitCallback = 0;

/* For some reaon, Apple removed setAppleMenu from the headers in 10.4,
 but the method still is there and works. To avoid warnings, we declare
 it ourselves here. */
@interface NSApplication(April_Missing_Methods)
- (void)setAppleMenu:(NSMenu*) menu;
@end

@interface AprilApplication : NSApplication
@end

@implementation AprilApplication
- (void)terminate:(id)sender
{
#ifdef _SDL
	gAprilShouldInvokeQuitCallback = 1;
#else
	if (sender == nil) // called from Window::terminateMainLoop()
	{
		[super terminate:sender];
	}
	else if (april::window->handleQuitRequest(true))
	{
		[super terminate:sender];
	}
	else
	{
		NSLog(@"Aborting application quit request per app's request.");
	}	
#endif
}

@end

NSString* getApplicationName()
{
	NSDictionary* dict;
	NSString* appName = 0;
	
	/* Determine the application name */
	appName = [[[NSBundle mainBundle] localizedInfoDictionary] objectForKey:@"CFBundleDisplayName"];
	if (!appName || [appName length] == 0)
	{
		dict = (NSDictionary*) CFBundleGetInfoDictionary(CFBundleGetMainBundle());
		if (dict)
			appName = [dict objectForKey: @"CFBundleName"];
		
		if (![appName length])
			appName = [[NSProcessInfo processInfo] processName];
	}
	return appName;
}

#ifdef _SDL // should be refactored for SDL

@interface AprilAppDelegate : NSObject<NSApplicationDelegate>
@end

/* The main class of the application, the application's delegate */
@implementation AprilAppDelegate

/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification*) note
{
	/* Hand off to main application code */
	//gCalledAppMainline = TRUE;
	harray<hstr> args;
	for (int i = 0; i < gArgc; i++)
	{
		args.push_back(gArgv[i]);
	}
	gAprilInit(args);
#ifdef _SDL
	april::window->enterMainLoop();
	gAprilDestroy();
	exit(0);
#endif
}

- (void) applicationWillTerminate:(NSNotification*) note
{
	gAprilDestroy();
}

#ifdef _COCOA_WINDOW

- (void)applicationDidBecomeActive:(NSNotification *)aNotification
{
	aprilWindow->OnAppGainedFocus();
}

- (void)applicationDidResignActive:(NSNotification *)aNotification
{
	aprilWindow->OnAppLostFocus();
}

#endif

@end

#endif

static NSString* getLocalizedString(NSString* key, NSString* fallback)
{
	NSString* s = [[[NSBundle mainBundle] localizedInfoDictionary] objectForKey:key];
	return (s == nil || [s length] == 0) ? fallback : s;
}

static void setApplicationMenu()
{
	/* warning: this code is very odd */
	NSMenu*     appleMenu;
	NSMenuItem* menuItem;
	NSString*   title;
	NSString*   appName;
	
	appName = getApplicationName();
	appleMenu = [[NSMenu alloc] initWithTitle:@""];
	
	/* Add menu items */
	title = [getLocalizedString(@"MenuAbout", @"About ") stringByAppendingString:appName];
	[appleMenu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
	
	[appleMenu addItem:[NSMenuItem separatorItem]];
	
	title = [getLocalizedString(@"MenuHide", @"Hide ") stringByAppendingString:appName];
	[appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];
	
	title = getLocalizedString(@"MenuHideOthers", @"Hide Others");
	menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:title action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
	[menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];
	
	title = getLocalizedString(@"MenuShowAll", @"Show All");
	[appleMenu addItemWithTitle:title action:@selector(unhideAllApplications:) keyEquivalent:@""];
	
	[appleMenu addItem:[NSMenuItem separatorItem]];
	
	title = [getLocalizedString(@"MenuQuit", @"Quit ") stringByAppendingString:appName];
	[appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];
	
	
	/* Put menu into the menubar */
	menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
	[menuItem setSubmenu:appleMenu];
	[[NSApp mainMenu] addItem:menuItem];
	
	/* Tell the application object that this is now the application menu */
	[NSApp setAppleMenu:appleMenu];
	
	/* Finally give up our references to the objects */
	[appleMenu release];
	[menuItem release];
}

/* Replacement for NSApplicationMain */
static void CustomApplicationMain(int argc, char **argv)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	AprilAppDelegate* appDelegate;
	
	/* Ensure the application object is initialised */
	[AprilApplication sharedApplication];

#ifdef SDL_USE_CPS
	{
		CPSProcessSerNum PSN;
		/* Tell the dock about us */
		if (!CPSGetCurrentProcess(&PSN))
			if (!CPSEnableForegroundOperation(&PSN,0x03,0x3C,0x2C,0x1103))
				if (!CPSSetFrontProcess(&PSN))
					[SDLApplication sharedApplication];
	}
#endif /* SDL_USE_CPS */
	
	/* Set up the menubar */
	[NSApp setMainMenu:[[NSMenu alloc] init]];
	setApplicationMenu();
	
	/* Create AprilAppDelegate and make it the app delegate */
	appDelegate = [[AprilAppDelegate alloc] init];
	[NSApp setDelegate:appDelegate];
	[NSApp activateIgnoringOtherApps:YES];
	
	/* Start the main event loop */
	[NSApp run];
	
	[appDelegate release];
	[pool release];
}


/* Main entry point to executable - should *not* be SDL_main! */
int april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char **argv)
{
	/* Copy the arguments into a global variable */
	/* This is passed if we are launched by double-clicking */
	
	gAprilShouldInvokeQuitCallback = 0;
	
	gAprilInit = anAprilInit;
	gAprilDestroy = anAprilDestroy;
	
	if (argc >= 2 && strncmp(argv[1], "-psn", 4) == 0)
	{
		gArgv = (char**) malloc(sizeof(char*) * 2);
		gArgv[0] = argv[0];
		gArgv[1] = NULL;
		gArgc = 1;
		gFinderLaunch = YES;
	}
	else
	{
		int i;
		gArgc = argc;
		gArgv = (char**) malloc(sizeof(char*) * (argc + 1));
		for (i = 0; i <= argc; i++)
			gArgv[i] = argv[i];
		gFinderLaunch = NO;
	}
	CustomApplicationMain(argc, argv);
	return 0;
}
