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
/// Defines all functions used in aprilui.

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>

#import "EAGLView.h"
#include "iOS_Window.h"
#include "RenderSystem.h"
#include "april.h"
#include "Keys.h"


#define USE_DEPTH_BUFFER 0
#define aprilWindow ((april::iOS_Window*) april::window)


// A class extension to declare private methods
@interface EAGLView ()

@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, assign) NSTimer *animationTimer;

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end



@implementation EAGLView

@synthesize context;
@synthesize animationTimer;

// You must implement this method
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}


// Handles the start of a touch
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	if (april::window)
		aprilWindow->touchesBegan_withEvent_(touches, event);
}

// Handles the end of a touch event when the touch is a tap.
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	if (april::window)
		aprilWindow->touchesEnded_withEvent_(touches, event);
}

// Called if touches are cancelled and need to be undone. 
// On iPhone, happens when 5 fingers are pressed.
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	if (april::window)
		aprilWindow->touchesCancelled_withEvent_(touches, event);
}

// Handles the movement of a touch event. 
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	if (april::window)
		aprilWindow->touchesMoved_withEvent_(touches, event);
}

- (id)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame]))
	{
		app_started = 0;
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
		
#if __IPHONE_3_2 //__IPHONE_OS_VERSION_MIN_REQUIRED >= 30200
		if ([eaglLayer respondsToSelector:@selector(setContentsScale:)])
		{
			if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
			{ // iphone 4
				eaglLayer.contentsScale = [[UIScreen mainScreen] scale];
			}
		}
#endif
		self.multipleTouchEnabled = YES;
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
#ifdef _OPENGLES1
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
#else
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
#endif
        if (!context || ![EAGLContext setCurrentContext:context])
		{
            [self release];
            return nil;
        }
        
        animationInterval = 1.0 / 60.0;
		
		
		// fake textbox
		CGRect textFrame = frame;
		textFrame.origin.x += frame.size.width;
		textField = [[UITextField alloc] initWithFrame:textFrame];
		textField.delegate = self;
		textField.text = @" "; // something to be able to catch text edit
		textField.autocapitalizationType = UITextAutocapitalizationTypeNone;
		textField.autocorrectionType = UITextAutocorrectionTypeNo;
		textField.keyboardAppearance = UIKeyboardAppearanceDefault;
		textField.keyboardType = UIKeyboardTypeDefault;
		textField.returnKeyType = UIReturnKeyDone;
		textField.secureTextEntry = NO;
		[self addSubview:textField];

		// tracking of keyboard appearance/disappearance
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(keyboardWasShown:)
													 name:UIKeyboardDidShowNotification object:nil];
		
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(keyboardWasHidden:)
													 name:UIKeyboardDidHideNotification object:nil];
		// tracking of device orientation change
		// we call UIDevice's beginGeneratingDeviceOrientationNotifications
		// in window itself 
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(deviceOrientationDidChange:)
													 name:UIDeviceOrientationDidChangeNotification 
												   object:nil];
		
		[textField addTarget:self
						   action:@selector(textFieldFinished:)
							forControlEvents:UIControlEventEditingDidEndOnExit];
		// create ios window object
		april::init(april::RS_DEFAULT, april::WS_DEFAULT);
		april::createRenderSystem();
		april::createWindow(0, 0, 1, "iOS Window");
		april::rendersys->assignWindow(april::window);
    }
	
    return self;
}

- (BOOL)textFieldShouldReturn:(UITextField *)aTextField
{
    [textField resignFirstResponder];
	if (april::window)
	{
		aprilWindow->handleKeyEvent(april::Window::KEY_DOWN, april::AK_RETURN, 0);
		aprilWindow->handleKeyEvent(april::Window::KEY_UP, april::AK_RETURN, 0);
	}
    return YES;
}

// we'll also use this objc class for getting notifications
// on virtual keyboard's appearance and disappearance

- (void)keyboardWasShown:(NSNotification*)notification
{
	if (april::window)
	{
		NSDictionary* info = [notification userInfo];
		CGSize kbSize = [[info objectForKey:UIKeyboardFrameBeginUserInfoKey] CGRectValue].size;
		CGSize screenSize = [[UIScreen mainScreen] bounds].size;
		aprilWindow->keyboardWasShown(kbSize.width / screenSize.width);
	}
}

- (void)keyboardWasHidden:(id)sender
{
	if (april::window)
		aprilWindow->keyboardWasHidden();
}

-(void)deviceOrientationDidChange:(id)sender
{

}

// ok, now other functionality of this class

- (void)beginRender
{
    if (!self.animationTimer)
	{
		NSLog(@"Called drawView while in background!");
		return;
	}
 //   glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer); // commented this out on June 8th 2012, it's probably reduntant, but I'll keep it here for a while just in case. -- kspes
    
    [EAGLContext setCurrentContext:context];
}

- (void)drawView
{
    [self beginRender];
	
	//mydraw();
	if (april::window)
		aprilWindow->handleDisplayAndUpdate();
	


    //[self swapBuffers];
}


-(void)_paintRect:(GLfloat[])vertices
{
	
}


- (void)swapBuffers
{
	if (!self.animationTimer)
	{
		NSLog(@"Warning: OpenGL swapBuffers while app in background, ignoring!");
		return;
	}
//	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer); // commented this out on June 8th 2012, it's probably reduntant, but I'll keep it here for a while just in case. -- kspes
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];

}

- (void)layoutSubviews
{
	CGRect textFrame = textField.frame;
	textFrame.origin.x += textFrame.size.width;
	textField.frame = textFrame;
	
    [EAGLContext setCurrentContext:context];
    [self destroyFramebuffer];
    [self createFramebuffer];
}

#ifdef _OPENGLES2
GLuint _positionSlot;
GLuint _colorSlot;

- (GLuint)compileShader:(NSString*)shaderString withType:(GLenum)shaderType {
	
    // 2
    GLuint shaderHandle = glCreateShader(shaderType);
	
    // 3
    const char * shaderStringUTF8 = [shaderString UTF8String];
    int shaderStringLength = [shaderString length];
    glShaderSource(shaderHandle, 1, &shaderStringUTF8, &shaderStringLength);
	
    // 4
    glCompileShader(shaderHandle);
	
    // 5
    GLint compileSuccess;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
    if (compileSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        NSString *messageString = [NSString stringWithUTF8String:messages];
        NSLog(@"%@", messageString);
        exit(1);
    }
	
    return shaderHandle;
	
}

- (void)compileShaders {
	const char* vert = "\
attribute vec4 Position;\
void main(void) {\
	gl_Position = Position;\
}";
	const char* frag = "\
void main(void) {\
	gl_FragColor = vec4(1,0.8,0,1);\
}";
    // 1
    GLuint vertexShader = [self compileShader:[NSString stringWithUTF8String:vert]
									 withType:GL_VERTEX_SHADER];
    GLuint fragmentShader = [self compileShader:[NSString stringWithUTF8String:frag]
									   withType:GL_FRAGMENT_SHADER];
	
    // 2
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);
    glLinkProgram(programHandle);
	
    // 3
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        NSString *messageString = [NSString stringWithUTF8String:messages];
        NSLog(@"%@", messageString);
        exit(1);
    }
	
    // 4
    glUseProgram(programHandle);
	
    // 5
    _positionSlot = glGetAttribLocation(programHandle, "Position");
  //  _colorSlot = glGetAttribLocation(programHandle, "SourceColor");
    glEnableVertexAttribArray(_positionSlot);
    //glEnableVertexAttribArray(_colorSlot);
}
#endif

- (BOOL)createFramebuffer
{
    glGenFramebuffersOES(1, &viewFramebuffer);
    glGenRenderbuffersOES(1, &viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
	
	NSString *depth = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"april_depth_buffer"];
	
    if (depth != nil)
	{
        glGenRenderbuffersOES(1, &depthRenderbuffer);
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
        glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
    }
    
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
	{
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
	// clear crap from previous renders. I often got a magenta colored initial screen without this
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, backingWidth, backingHeight);
	
#ifdef _OPENGLES2
	[self compileShaders];
#endif
    return YES;
}


- (void)destroyFramebuffer
{
    glDeleteFramebuffersOES(1, &viewFramebuffer);
    viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &viewRenderbuffer);
    viewRenderbuffer = 0;
    
    if(depthRenderbuffer)
	{
        glDeleteRenderbuffersOES(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
}


- (void)startAnimation
{
	if (!app_started) return;
    self.animationTimer = [NSTimer scheduledTimerWithTimeInterval:animationInterval target:self selector:@selector(drawView) userInfo:nil repeats:YES];
}


- (void)stopAnimation
{
    self.animationTimer = nil;
}


- (void)setAnimationTimer:(NSTimer *)newTimer
{
    if(animationTimer != newTimer)
		[animationTimer invalidate];
    animationTimer = newTimer;
}


- (void)setAnimationInterval:(NSTimeInterval)interval
{
    
    animationInterval = interval;
    if (animationTimer)
	{
        [self stopAnimation];
        [self startAnimation];
    }
}

- (NSTimeInterval)setAnimationInterval
{
	return animationInterval;
}

- (void)dealloc
{
    
    [self stopAnimation];
    
    if ([EAGLContext currentContext] == context)
	{
        [EAGLContext setCurrentContext:nil];
    }
    
    [context release];  
    [super dealloc];
}

- (void)beginKeyboardHandling
{
	[textField becomeFirstResponder];
}

- (BOOL)isKeyboardActive
{
	return [textField isFirstResponder];
}

- (void)terminateKeyboardHandling
{
	[textField endEditing:YES];
}

- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string   // return NO to not change text
{
	if (april::window == NULL) return NO;
	hstr str = chstr([string UTF8String]);
	if (range.location == 0 && str.size() == 0) aprilWindow->injectiOSChar(0); // backspace indicator
	else if (str.size() == 0) return NO;
	unichar chars[256];
	[string getCharacters:chars];
	int len = [string length];
	
	for (int i = 0; i < len; i++)
	{
		aprilWindow->injectiOSChar(chars[i]);
	}
	return NO;
}

-(void)applicationDidBecomeActive:(UIApplication*)app
{
	if (april::window)
		aprilWindow->applicationDidBecomeActive();
}

-(void)applicationWillResignActive:(UIApplication*)app
{
	if (april::window)
		aprilWindow->applicationWillResignActive();
}

@end
