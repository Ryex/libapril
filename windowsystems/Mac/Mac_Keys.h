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
/// 
/// Key definition converter.

#ifndef APRIL_MAC_KEYS_H
#define APRIL_MAC_KEYS_H

#include "Keys.h"

namespace april
{
	Key getAprilMacKeyCode(unsigned int macKeyCode);
	void initMacKeyMap();
}

#endif
