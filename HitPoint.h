#pragma once

class Point3D;
class Normal;
class RGBColor;
class Material;

class HitPoint
{
public:
	bool hit;				// If hit an object
	Point3D point;			// Hit point in 3D
	Normal normal;			// Object normal at hit point
	float ray_t;			// Ray.t parameter
	float u;				// 2D Texture coordinates
	float v;				// 2D Texture coordinates
	RGBColor color;			// Color of the material
	RGBColor color_light;	// Colors from lights
	float intensity_light;	// Intensities of lights
	Material* material;		// Material pointer of the hit object
	int ray_counter;		// The counter indicating # of intersection test
	// Defult constructor, black color with no lighting
	HitPoint();
	// Copy constructor
	HitPoint(const HitPoint& hitPoint);
	// Assign operator
	HitPoint& operator= (const HitPoint& rhs);
	~HitPoint();
	// Lighting Resetter
	void reset_light();
	// Get the material color according to HitPoint::UV
	RGBColor get_color() const;
};