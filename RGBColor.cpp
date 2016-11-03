#include <math.h>
#include "precomp.h"
#include "RGBColor.h"



// Destructor
RGBColor::~RGBColor()
{
}

// Assignment
RGBColor& RGBColor::operator=  (const RGBColor& color)
{
	if (this == &color)
		return *this;
	r = color.r;
	g = color.g;
	b = color.b;
	return *this;
}
// Addition
RGBColor  RGBColor::operator+  (const RGBColor& color) const
{
	return RGBColor(r + color.r, g + color.g, b + color.b);
}
// Compound Addition
RGBColor& RGBColor::operator+= (const RGBColor& color)
{
	r += color.r;
	g += color.g;
	b += color.b;
	return *this;
}
// Multiplication by a float on the right
RGBColor  RGBColor::operator*  (const float value) const
{
	return RGBColor(r*value, g*value, b*value);
}
// Compund Multiplication
RGBColor& RGBColor::operator*= (const float value)
{
	r *= value;
	g *= value;
	b *= value;
	return *this;
}
// Division by a float
RGBColor  RGBColor::operator/  (const float value) const
{
	float valueInv = float(1.0 / value);
	return RGBColor(r*valueInv, g*valueInv, b*valueInv);
}
// Compound Division
RGBColor& RGBColor::operator/= (const float value)
{
	float valueInv = float(1.0 / value);
	r *= valueInv;
	g *= valueInv;
	b *= valueInv;
	return *this;
}
// (R1*R2, G1*G2, B1*B2) by two RGBColor
RGBColor  RGBColor::operator* (const RGBColor& color) const
{
	return RGBColor(r*color.r, g*color.g, b*color.b);
}
// Returns if two RGBColor are equal
bool RGBColor::operator== (const RGBColor& color) const
{
	return (r == color.r && g == color.g && b == color.b);
}
// Raise R,G,B to a power
RGBColor RGBColor::powColor(float power) const
{
	return RGBColor(pow(r, power), pow(g, power), pow(b, power));
}
// Two simple Gamma correction methods, for explicit gamma value, use powColor()
void RGBColor::correct_gamma()
{
	this->r = sqrtf(this->r);
	this->g = sqrtf(this->g);
	this->b = sqrtf(this->b);
}
void RGBColor::un_correct_gamma()
{
	this->r = this->r * this->r;
	this->g = this->g * this->g;
	this->b = this->b * this->b;
}
// Return the average color value
float RGBColor::average() const
{
	return float((r + g + b)*0.333333333333);
}
// Output a Pixel color for SDL to use
long RGBColor::out() const
{
	long color;
	float R = r > 1.0f ? 1.0f : r;
	float G = g > 1.0f ? 1.0f : g;
	float B = b > 1.0f ? 1.0f : b;
	color = (int(R * 255) << 16) + (int(G * 255) << 8) + (int(B * 255));
	return color;
}

// Non-member functions
// Multiplication by a float on the left
RGBColor operator* (const float value, const RGBColor& color)
{
	return RGBColor(value * color.r, value * color.g, value * color.b);
}

/// colorBuffer methods
colorBuffer::colorBuffer()
{
	this->reset_buffer();
}
// Add a pixel color to buffer
void colorBuffer::add_to_buffer(RGBColor color, int x, int y)
{
	this->buffer[x][y] += color;
}
// Get pixel color
RGBColor colorBuffer::get_color(int x, int y)
{
	return this->buffer[x][y] / (float)this->counter;
}
// Reset Buffer
void colorBuffer::reset_buffer()
{
	// Reset counter to 0, and all colors in buffer to black
	this->counter = 0;
#pragma omp parallel for
	for (int x = 0; x < SCRWIDTH; x++)
	{
#pragma omp parallel for
		for (int y = 0; y < SCRHEIGHT; y++)
		{
			buffer[x][y] = RGBColor();
		}
	}
}