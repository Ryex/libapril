/// @file
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "VertexShader.h"

namespace april
{
	VertexShader::VertexShader()
	{
	}

	VertexShader::~VertexShader()
	{
	}

	bool VertexShader::_loadData(chstr filename, unsigned char** data, long* size)
	{
		if (!hresource::exists(filename))
		{
			return false;
		}
		hresource stream(filename);
		*size = stream.size();
		*data = new unsigned char[*size];
		stream.read_raw(*data, *size);
		return true;
	}
	
}
