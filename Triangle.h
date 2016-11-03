#pragma once

#include "GeoPrimitive.h"
#include "Point3D.h"
#include "Normal.h"
#include "RGBColor.h"
#include "HitPoint.h"
#include "AABB.h"

class Triangle :
	public GeoPrimitive
{
protected:
	// Three vertices
	Point3D v0, v1, v2;
	// The face normal
	Normal n;
	// Color
	RGBColor color;
public:
	// Constructors
	// Defult Constructor, gives a triangle on X-Z plane
	Triangle();
	// Construct from three Point3D
	Triangle(	const Point3D& p0,
				const Point3D& p1,
				const Point3D& p2	);
	// Copy Constructor
	Triangle(const Triangle& triangle);
	// Destructor
	~Triangle();

	// Assignment operator
	Triangle& operator= (const Triangle& triangle);
	// Normal computer
	void compute_normal();
	// Get normal
	virtual Normal get_normal(const Point3D& p3d) { return this->n; }
	// Set color
	virtual void set_color(const RGBColor& colorInput);
	// Color getter
	virtual RGBColor get_color();
	// Get Bounding Box
	virtual AABB get_AABB() const;
	// Get Triangle Area
	virtual float get_area() const;
	// Generate a random Point3D on surface
	virtual Point3D rand_pnt(const Point3D& p3d);
	// Return a Point3D corresponding to the Point2D Barycentric coordinates
	Point3D get_uv(const Point2D& p2d);
	// Ray-Triangle hit function
	virtual bool hit(Ray& ray) const;
	// Ray-Triangle hit function, with RGBColor
	virtual bool hit(Ray& ray, RGBColor& colorOutput) const;
	// Ray-Triangle hit function, with HitPoint
	virtual bool hit(Ray& ray, HitPoint& hitPoint) const;
	// Ray-Triangle hit function, Moller method <- not correct
	bool hit_moller(Ray& ray) const;
};