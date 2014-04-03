/// @file
/// @author  Kresimir Spes
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
/// Defines a generic timer.

#ifndef APRIL_TIMER_H
#define APRIL_TIMER_H

#include <hltypes/hplatform.h>
#ifndef _WIN32
#include <stdint.h>
#define __int64 uint64_t
#endif
#include <stdio.h>

#include "aprilExport.h"

namespace april
{
	class aprilExport Timer
	{
	public:
		Timer();
		~Timer();
		
		float getTime();
		float diff(bool update = true);
		
		void update();
		
	protected:
		float dt;
		float td;
		float td2;
		__int64 frequency;
		float resolution;
		__int64 mTimerStart;
		unsigned long mTimerElapsed;
		bool performanceTimer;
		__int64 performanceTimerStart;
		__int64 performanceTimerElapsed;
		
	};
}

#endif
