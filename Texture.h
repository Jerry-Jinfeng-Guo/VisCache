#pragma once

class RGBColor;

class Texture
{
public:
	Texture(){}
	~Texture(){}
	virtual RGBColor value(const float u, const float v, const Point3D& p3d) = 0;
};

class ConstTex : public Texture
{
public:
	ConstTex(){ this->color = RGBColor(); }
	ConstTex(const RGBColor& colorInput){ this->color = colorInput; }
	~ConstTex(){}

	virtual RGBColor value(const float u, const float v, const Point3D& p3d)
	{
		return this->color;
	}
private:
	RGBColor color;
};

class CheckerTex : public Texture
{
public:
	CheckerTex() { checkerSize = 0.5f; light = RGBColor(0.8f); dark = RGBColor(0.1f); }
	CheckerTex(float ckrSize, RGBColor lightInput, RGBColor darkInput)
		: checkerSize(ckrSize), light(lightInput), dark(darkInput) {}
	~CheckerTex() {}
	virtual RGBColor value(const float u, const float v, const Point3D& p3d);
private:
	RGBColor light, dark;
	float checkerSize;
};

// A Perlin Noise implementation according to Andrew Kensler's description
class Perlin
{
public:
	Perlin();
	~Perlin(){}
	float turb(const Point3D& p3d) const;
	float noise(const Point3D& p3d) const;
	Vector3D *ranVec;
	int *perm_x;
	int *perm_y;
	int *perm_z;
};

class MarbleTex : public Texture
{
public:
	MarbleTex(){ scale = 1.0f; color = RGBColor(4.5f); }
	MarbleTex(float noiseScale) : scale(noiseScale), color(RGBColor(4.5f)){}
	MarbleTex(float noiseScale, RGBColor& c) : scale(noiseScale), color(c){}
	~MarbleTex(){}
	virtual RGBColor value(const float u, const float v, const Point3D& p3d)
	{
		float c = (1.0f + sinf(scale*p3d.z + noise.turb(scale*p3d) * 10.0f));
		return sqrtf(c)*this->color;
	}
private:
	Perlin noise;
	float scale;
	RGBColor color;
};

struct FIBITMAP;

class ImageTex : public Texture
{
public:
	ImageTex(){}
	// Construct from an image file
	ImageTex(const char* filePath);
	~ImageTex(){}
	// Get pixel color according to pixel coordinate, here U and V are used
	virtual RGBColor value(const float u, const float v, const Point3D& p3d);
	// Get Width and height
	uint32_t get_w(){ return (uint32_t)width; }
	uint32_t get_h(){ return (uint32_t)height; }
private:
	RGBColor image[4096][4096];
	unsigned width;
	unsigned height;
};

// Non-member functions
static Vector3D* perlin_generate();
void permute(int *p, int n);
static int* perlin_generate_perm();
float trilinear_initerp(float c[2][2][2], float u, float v, float w);
float perlin_initerp(Vector3D c[2][2][2], float u, float v, float w);