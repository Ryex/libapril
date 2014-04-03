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
/// Microsoft believes that forcing developers to have a pch.h file is a good idea and especially useful for those
/// who must use XAML + DirectX interop in order to display a virtual keyboard when text input is required.

#ifndef APRIL_WINRT_PCH_H
#define APRIL_WINRT_PCH_H

#include <hltypes/hstring.h> // because "ppltasks.h" messes with std::string so hltypes::String needs to be declared first
#include <ppltasks.h>

#endif