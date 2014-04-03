/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.34
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdio.h>

#include <hltypes/exception.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Color.h"

namespace april
{
	// predefined colors
	Color Color::White(255, 255, 255);
	Color Color::Black(0, 0, 0);
	Color Color::Grey(127, 127, 127);
	Color Color::Red(255, 0, 0);
	Color Color::Green(0, 255, 0);
	Color Color::Blue(0, 0, 255);
	Color Color::Yellow(255, 255, 0);
	Color Color::Magenta(255, 0, 255);
	Color Color::Cyan(0, 255, 255);
	Color Color::Orange(255, 127, 0);
	Color Color::Pink(255, 0, 127);
	Color Color::Teal(0, 255, 127);
	Color Color::Neon(127, 255, 0);
	Color Color::Purple(127, 0, 255);
	Color Color::Aqua(0, 127, 255);
	Color Color::DarkGrey(63, 63, 63);
	Color Color::DarkRed(127, 0, 0);
	Color Color::DarkGreen(0, 127, 0);
	Color Color::DarkBlue(0, 0, 127);
	Color Color::DarkYellow(127, 127, 0);
	Color Color::DarkMagenta(127, 0, 127);
	Color Color::DarkCyan(0, 127, 127);
	Color Color::DarkOrange(127, 63, 0);
	Color Color::DarkPink(127, 0, 63);
	Color Color::DarkTeal(0, 127, 63);
	Color Color::DarkNeon(63, 127, 0);
	Color Color::DarkPurple(63, 0, 127);
	Color Color::DarkAqua(0, 63, 127);
	Color Color::Clear(0, 0, 0, 0);
	Color Color::Blank(255, 255, 255, 0);
	
	Color::Color()
	{
		this->r = 255;
		this->g = 255;
		this->b = 255;
		this->a = 255;
	}
	
	Color::Color(int r, int g, int b, int a)
	{
		this->set(r, g, b, a);
	}
	
	Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		this->set(r, g, b, a);
	}
	
	Color::Color(unsigned int color)
	{
		this->set(color);
	}
	
	Color::Color(chstr hex)
	{
		this->set(hex);
	}
	
	Color::Color(const char* hex)
	{
		this->set(hstr(hex));
	}
	
	Color::Color(const Color& color, unsigned char a)
	{
		this->set(color, a);
	}
	
	void Color::set(int r, int g, int b, int a)
	{
		this->r = (unsigned char)r;
		this->g = (unsigned char)g;
		this->b = (unsigned char)b;
		this->a = (unsigned char)a;
	}
	
	void Color::set(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
	
	void Color::set(unsigned int color)
	{
		this->r = (unsigned char)((color >> 24) & 0xFF);
		this->g = (unsigned char)((color >> 16) & 0xFF);
		this->b = (unsigned char)((color >> 8) & 0xFF);
		this->a = (unsigned char)(color & 0xFF);
	}
	
	void Color::set(chstr hex)
	{
		hstr value = (hex.starts_with("0x") ? hex(2, -1) : hex);
		if (value.size() != 6 && value.size() != 8 && !value.is_hex())
		{
			throw hl_exception("Color format must be either 0xRRGGBBAA or 0xRRGGBB (with or without 0x prefix)");
		}
		this->r = (unsigned char)value(0, 2).unhex();
		this->g = (unsigned char)value(2, 2).unhex();
		this->b = (unsigned char)value(4, 2).unhex();
		this->a = (value.size() == 8 ? (unsigned char)value(6, 2).unhex() : 255);
	}
	
	void Color::set(const char* hex)
	{
		this->set(hstr(hex));
	}
	
	void Color::set(const Color& color, unsigned char a)
	{
		this->r = color.r;
		this->g = color.g;
		this->b = color.b;
		this->a = a;
	}
	
	hstr Color::hex(bool rgbOnly) const
	{
		return (!rgbOnly ? hsprintf("%02X%02X%02X%02X", this->r, this->g, this->b, this->a) : hsprintf("%02X%02X%02X", this->r, this->g, this->b));
	}
	
	Color::operator unsigned int() const
	{
		return ((this->r << 24) | (this->g << 16) | (this->b << 8) | this->a);
	}
	
	bool Color::operator==(const Color& other) const
	{
		return (this->r == other.r && this->g == other.g && this->b == other.b && this->a == other.a);
	}
	
	bool Color::operator!=(const Color& other) const
	{
		return (this->r != other.r || this->g != other.g || this->b != other.b || this->a != other.a);
	}
	
	Color Color::operator+(const Color& other) const
	{
		Color result(*this);
		result += other;
		return result;
	}
	
	Color Color::operator-(const Color& other) const
	{
		Color result(*this);
		result -= other;
		return result;
	}
	
	Color Color::operator*(const Color& other) const
	{
		Color result(*this);
		result *= other;
		return result;
	}
	
	Color Color::operator/(const Color& other) const
	{
		Color result(*this);
		result /= other;
		return result;
	}
	
	Color Color::operator*(float value) const
	{
		Color result(*this);
		result *= value;
		return result;
	}
	
	Color Color::operator/(float value) const
	{
		Color result(*this);
		result /= value;
		return result;
	}
	
	Color Color::operator+=(const Color& other)
	{
		this->r = (unsigned char)hclamp((int)this->r + other.r, 0, 255);
		this->g = (unsigned char)hclamp((int)this->g + other.g, 0, 255);
		this->b = (unsigned char)hclamp((int)this->b + other.b, 0, 255);
		this->a = (unsigned char)hclamp((int)this->a + other.a, 0, 255);
		return (*this);
	}
	
	Color Color::operator-=(const Color& other)
	{
		this->r = (unsigned char)hclamp((int)this->r - other.r, 0, 255);
		this->g = (unsigned char)hclamp((int)this->g - other.g, 0, 255);
		this->b = (unsigned char)hclamp((int)this->b - other.b, 0, 255);
		this->a = (unsigned char)hclamp((int)this->a - other.a, 0, 255);
		return (*this);
	}
	
	Color Color::operator*=(const Color& other)
	{
		this->r = (unsigned char)hclamp((int)(this->r_f() * other.r), 0, 255);
		this->g = (unsigned char)hclamp((int)(this->g_f() * other.g), 0, 255);
		this->b = (unsigned char)hclamp((int)(this->b_f() * other.b), 0, 255);
		this->a = (unsigned char)hclamp((int)(this->a_f() * other.a), 0, 255);
		return (*this);
	}
	
	Color Color::operator/=(const Color& other)
	{
		this->r = (unsigned char)hclamp((int)(this->r_f() / other.r), 0, 255);
		this->g = (unsigned char)hclamp((int)(this->g_f() / other.g), 0, 255);
		this->b = (unsigned char)hclamp((int)(this->b_f() / other.b), 0, 255);
		this->a = (unsigned char)hclamp((int)(this->a_f() / other.a), 0, 255);
		return (*this);
	}
	
	Color Color::operator*=(float value)
	{
		this->r = (unsigned char)hclamp((int)(this->r * value), 0, 255);
		this->g = (unsigned char)hclamp((int)(this->g * value), 0, 255);
		this->b = (unsigned char)hclamp((int)(this->b * value), 0, 255);
		this->a = (unsigned char)hclamp((int)(this->a * value), 0, 255);
		return (*this);
	}
	
	Color Color::operator/=(float value)
	{
		float val = 1.0f / value;
		this->r = (unsigned char)hclamp((int)(this->r * val), 0, 255);
		this->g = (unsigned char)hclamp((int)(this->g * val), 0, 255);
		this->b = (unsigned char)hclamp((int)(this->b * val), 0, 255);
		this->a = (unsigned char)hclamp((int)(this->a * val), 0, 255);
		return (*this);
	}
	
}
