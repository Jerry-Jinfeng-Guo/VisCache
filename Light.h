#pragma once

class Point3D;
class Vector3D;
class RGBColor;

class Light
{
public:
	// Constructors
	// Defult constructor
	Light();
	// Construct from Point3D, RGBColor and float value
	Light(const Point3D& p3d, const RGBColor& color, const float& intensityInput);
	// Copy Constructor
	Light(const Light& lightInput);
	// Destructor
	~Light();
	// Get light position
	Point3D get_position();
	// Get light color
	RGBColor get_color();
	// Get intensity given a 3D point, flag indicating decay type
	float get_intensity(const Point3D& p3d, int flag);
	// Get full intensity
	float get_fullIntensity();
protected:
	// Light source position
	Point3D p;
	// Light source color
	RGBColor color;
	// Light intensity
	float intensity;
};

