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
/// Defines some methods and special macros used with April's AndroidJNI system.

#ifdef _ANDROID
#ifndef ANDROID_UTIL_JNI_H
#define ANDROID_UTIL_JNI_H

#ifndef JNI_H_
#include "jni.h"
#endif

#include <hltypes/hstring.h>

namespace april
{
	JNIEnv* getJNIEnv();
	jobject getActivity();
	jobject getAprilActivity();
	jclass findJNIClass(JNIEnv* env, hstr classPath);
	hstr _jstringToHstr(JNIEnv* env, jstring string);
}

#define _JARGS(returnType, arguments) "(" arguments ")" returnType
#define _JARR(str) "[" str
#define _JOBJ "Ljava/lang/Object;"
#define _JSTR "Ljava/lang/String;"
#define _JCLASS(classe) "L" classe ";"
#define _JINT "I"
#define _JBOOL "Z"
#define _JFLOAT "F"
#define _JVOID "V"

#define _JSTR_TO_HSTR(string) april::_jstringToHstr(env, string)

#ifdef __NATIVE_INTERFACE_CLASS
#define APRIL_GET_NATIVE_INTERFACE_CLASS(className) \
	JNIEnv* env = april::getJNIEnv(); \
	jclass className = april::findJNIClass(env, __NATIVE_INTERFACE_CLASS);
#define APRIL_GET_NATIVE_INTERFACE_METHOD(className, methodName, methodString, args) \
	APRIL_GET_NATIVE_INTERFACE_CLASS(className); \
	jmethodID methodName = env->GetStaticMethodID(className, methodString, args);
#define APRIL_GET_NATIVE_INTERFACE_FIELD(className, fieldName, fieldString, type) \
	APRIL_GET_NATIVE_INTERFACE_CLASS(className); \
	jfieldID fieldName = env->GetStaticFieldID(className, fieldString, type);
#endif

#endif
#endif
