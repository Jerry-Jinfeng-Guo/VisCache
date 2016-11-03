#pragma once

#include "GeoPrimitive.h"
#include "Point3D.h"
#include "RGBColor.h"
#include "HitPoint.h"

class Sphere :
	public GeoPrimitive
{
protected:
	// Center Point3D
	Point3D c;
	// float radius
	float r;
	// float radius squared
	float r2;
	// Color
	RGBColor color;

public:
	// Constructors
	// Defult Constructor, gives a radius 1 grey sphere centered at the orgin
	Sphere();
	// Construct from a Point3D, a float radius and a RGBColor
	Sphere(const Point3D& center, const float& radius, RGBColor& colorInput);
	// Copy Constructor
	Sphere(const Sphere& sphereInput);
	// Destructor
	~Sphere();

	// Assignment operator
	Sphere& operator= (const Sphere& sphereInput);
	// Color setter
	virtual void set_color(const RGBColor& colorInput);
	// Get color
	virtual RGBColor get_color();
	// Get Bounding Box
	AABB get_AABB() const;
	// Get normal
	virtual Normal get_normal(const Point3D& p3d);
	// Get the HemiSphere area, notice here only half is returned!
	virtual float get_area() const { return TWO_PI * r2; };
	// Generate a random Point3D on surface
	virtual Point3D rand_pnt(const Point3D& p3d);
	// Ray-Sphere hit function
	virtual bool hit(Ray& ray) const;
	// Ray-Sphere hit function, another hit method, not yet optimized
	bool Sphere::hit2(Ray& ray) const;
	// Ray-Sphere hit function, with color
	virtual bool hit(Ray& ray, RGBColor& color) const;
	// Ray-Sphere hit function, with hit point
	virtual bool hit(Ray& ray, HitPoint& hitPoint) const;
};