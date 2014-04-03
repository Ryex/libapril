/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.34
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a color object.

#ifndef APRIL_COLOR_H
#define APRIL_COLOR_H

#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	class aprilExport Color
	{
	public:
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
		
		Color();
		Color(int r, int g, int b, int a = 255);
		Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
		Color(unsigned int color);
		Color(chstr hex);
		Color(const char* hex);
		Color(const Color& color, unsigned char a);

		void set(int r, int g, int b, int a = 255);
		void set(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
		void set(unsigned int color);
		void set(chstr hex);
		void set(const char* hex);
		void set(const Color& color, unsigned char a);

		float r_f() const { return this->r * 0.003921569f; } // equals r / 255, multiplication is faster than division
		float g_f() const { return this->g * 0.003921569f; } // equals g / 255, multiplication is faster than division
		float b_f() const { return this->b * 0.003921569f; } // equals b / 255, multiplication is faster than division
		float a_f() const { return this->a * 0.003921569f; } // equals a / 255, multiplication is faster than division
		
		hstr hex(bool rgbOnly = false) const; // careful when using rgbOnly!
		
		operator unsigned int() const;
		
		bool operator==(const Color& other) const;
		bool operator!=(const Color& other) const;
		
		Color operator+(const Color& other) const;
		Color operator-(const Color& other) const;
		Color operator*(const Color& other) const;
		Color operator/(const Color& other) const;
		Color operator*(float value) const;
		Color operator/(float value) const;
		Color operator+=(const Color& other);
		Color operator-=(const Color& other);
		Color operator*=(const Color& other);
		Color operator/=(const Color& other);
		Color operator*=(float value);
		Color operator/=(float value);

		static Color White;
		static Color Black;
		static Color Grey;
		static Color Red;
		static Color Green;
		static Color Blue;
		static Color Yellow;
		static Color Magenta;
		static Color Cyan;
		static Color Orange;
		static Color Pink;
		static Color Teal;
		static Color Neon;
		static Color Purple;
		static Color Aqua;
		static Color DarkGrey;
		static Color DarkRed;
		static Color DarkGreen;
		static Color DarkBlue;
		static Color DarkYellow;
		static Color DarkMagenta;
		static Color DarkCyan;
		static Color DarkOrange;
		static Color DarkPink;
		static Color DarkTeal;
		static Color DarkNeon;
		static Color DarkPurple;
		static Color DarkAqua;
		static Color Clear;
		static Color Blank;
		
	};

}

#endif
