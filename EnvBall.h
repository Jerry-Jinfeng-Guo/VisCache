#pragma once

class RGBColor;
struct FIBITMAP;

class EnvBall
{
private:
	// The 2-D texture array
	RGBColor texture[4096][4096];
	unsigned width;
	unsigned height;

public:
	// Default constructor, does nothing
	EnvBall(){}
	// Construct from an image file
	EnvBall(const char* filePath);
	// Destructor
	~EnvBall();

	// Get pixel color according to pixel coordinate
	inline RGBColor get_color(uint32_t u, uint32_t v) {	return texture[u][v]; }

	// Get Width and height
	inline uint32_t get_w(){ return (uint32_t)width; }
	inline uint32_t get_h(){ return (uint32_t)height; }
};

