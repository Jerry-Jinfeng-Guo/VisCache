#include "precomp.h"
#include "Light.h"

// Defult constructor
Light::Light()
: p(Point3D(0.0f, 30.0f, 0.0f)), color(RGBColor(1.0f)), intensity(10.0f)
{
}
// Construct from Point3D, RGBColor and float value
Light::Light(const Point3D& p3d, const RGBColor& colorInput, const float& intensityInput)
: p(p3d), color(colorInput), intensity(intensityInput)
{
}
// Copy Constructor
Light::Light(const Light& lightInput)
: p(lightInput.p), color(lightInput.color), intensity(lightInput.intensity)
{
}
// Destructor
Light::~Light()
{
}
// Get light position
Point3D Light::get_position()
{
	return this->p;
}
// Get light color
RGBColor Light::get_color()
{
	return this->color;
}
// Get intensity given a 3D point, flag indicating decay type
float Light::get_intensity(const Point3D& p3d, int flag)
{
	// For simplicity now               <<<----------------------------------------
	return intensity;
}
// Get full intensity
float Light::get_fullIntensity()
{
	return intensity;
}
