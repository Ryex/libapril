/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an iOS app delegate.

#import <UIKit/UIKit.h>

// we could define this as returning "void", and use NSData* or
// char* for success argument, or NSError* or int+char* for the
// failure argument. however, future iOS releases, or even some
// completely different platform, might implement same handler,
// yet require different arguments.
//
// hence, void* for return value and void* for the argument can
// help keep the callback future proof. is that smart? dunno :)
//
// idea: this could perhaps be refactored as a "generic" 
//       callback?
// idea: should we nevertheless switch to platform-specific
//       C-based handlers? 
typedef void*(*AprilRemoteNotificationsHandler_t)(void *data);

@class AprilViewController;

@interface ApriliOSAppDelegate : NSObject<UIApplicationDelegate>
{
	UIWindow *uiwnd;
	AprilViewController *viewController;
	AprilRemoteNotificationsHandler_t onPushRegistrationSuccess;
	AprilRemoteNotificationsHandler_t onPushRegistrationFailure;
}

@property (nonatomic, retain) UIWindow *uiwnd;
@property (nonatomic, retain) AprilViewController *viewController;
@property (nonatomic, assign) AprilRemoteNotificationsHandler_t onPushRegistrationSuccess;
@property (nonatomic, assign) AprilRemoteNotificationsHandler_t onPushRegistrationFailure;
@end
