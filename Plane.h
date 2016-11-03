#pragma once

#include "GeoPrimitive.h"
#include "Point3D.h"
#include "Normal.h"
#include "RGBColor.h"
#include "HitPoint.h"

class Plane :
	public GeoPrimitive
{
protected:
	// The point
	Point3D p;
	// The normal
	Normal n;
	// The color of the plane
	RGBColor color;
	// The d
	float d;

public:
	// Constructors
	// Defult constructor, gives the x-z plane with gray color
	Plane();
	// Construct from a Point3D, a Normal and a RGBColor
	Plane(const Point3D& p3d, const Normal& nrml, const RGBColor& colorInput);
	// Copy Constructor
	Plane(const Plane& planeInput);
	// Destructor
	~Plane();

	// Assignment operator
	Plane& operator= (const Plane& planeInput);

	// Color setter
	virtual void set_color(const RGBColor& colorInput);
	// Color getter
	virtual RGBColor get_color();
	// Get AABB, return a INFINITE AABB
	virtual AABB get_AABB() const;
	// Get normal
	virtual Normal get_normal(const Point3D& p3d) { return this->n; }
	virtual float get_area() { return VERYBIG; }
	// Generate a random Point3D on surface
	virtual Point3D rand_pnt(const Point3D& p3d);
	// Ray-Plane hit function
	virtual bool hit(Ray& ray) const;
	// Ray-Plane hit function, with color
	virtual bool hit(Ray& ray, RGBColor& colorOut) const;
	// Ray-Plane hit function, with hit point
	virtual bool hit(Ray& ray, HitPoint& hitPoint) const;
};

