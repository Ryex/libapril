/// @file
/// @author  Ivan Vucica
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#import <UIKit/UIKit.h>

#import "main_base.h"

int april_main (void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char **argv)
{	
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	int retVal = UIApplicationMain(argc, argv, nil, @"ApriliOSAppDelegate");
    [pool release];
    return retVal;
}
