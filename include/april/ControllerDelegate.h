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
/// Defines a delegate for the controller input callbacks.

#ifndef APRIL_CONTROLLER_DELEGATE_H
#define APRIL_CONTROLLER_DELEGATE_H

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	class aprilExport ControllerDelegate
	{
	public:
		ControllerDelegate();
		virtual ~ControllerDelegate();

		virtual void onButtonDown(april::Button buttonCode);
		virtual void onButtonUp(april::Button buttonCode);
		// TODO - analog triggers and analog sticks

	};

}
#endif
