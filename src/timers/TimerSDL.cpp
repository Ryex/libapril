/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <SDL/SDL.h>

#include "RenderSystem.h"
#include "Timer.h"

namespace april
{
    Timer::Timer()
    {
        this->dt = 0;
        this->td2 = 0;
        this->td = 0;
        this->frequency = 0;
        this->performanceTimerStart = 0;
        this->resolution = 0; // unused in SDL timer
        this->mTimerStart = 0;
        this->mTimerElapsed = 0;
        performanceTimerElapsed = 0;
        performanceTimer = 0;
        
		// for sdl:
		performanceTimer = 0; // was: "false"
		this->mTimerStart = SDL_GetTicks();
		this->frequency	= 1000;
		this->mTimerElapsed = this->mTimerStart;
            
        
    }
    
    Timer::~Timer()
    {
        
    }
    
    float Timer::getTime()
    {        
		return( (float) ( SDL_GetTicks() - this->mTimerStart));
        
    }
    
    float Timer::diff(bool doUpdate)
    {
        if(doUpdate)
        {
            this->update();
            return this->dt;
        }
        else
            return this->dt;
        
    }
    
    void Timer::update()
    {
        this->td2 = getTime();
        this->dt = (this->td2-this->td) * 0.1f;
        this->td = this->td2;
    }
}
