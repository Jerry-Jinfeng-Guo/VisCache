#pragma once
#include "GeoPrimitive.h"
#include "Point3D.h"
#include "Normal.h"
#include "RGBColor.h"
#include "HitPoint.h"
#include "AABB.h"

class Rect :
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
	// Defult, construct a unit square at the X-Z plane, size 1, pointing +Y
	Rect();
	// Construct from three Point3D
	Rect(	Point3D p0,
			Point3D p1,
			Point3D p2	);
	// Copy Constructor
	Rect(const Rect& rect);
	// Destructor
	~Rect();

	// Assign operator
	Rect& operator=(const Rect& rect);
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
	// Get Rect area
	virtual float get_area() const;
	// Generate a random Point3D on surface
	virtual Point3D rand_pnt(const Point3D& p3d);
	// Return a Point3D corresponding to the Point2D Barycentric coordinates
	Point3D get_uv(const Point2D& p2d);
	// Calculate the UV of the HitPoint
	void cal_uv(HitPoint& hitPoint) const;
	// Ray-Rect hit function
	virtual bool hit(Ray& ray) const;
	// Ray-Rect hit function, with RGBColor
	virtual bool hit(Ray& ray, RGBColor& colorOutput) const;
	// Ray-Rect hit function, with HitPoint
	virtual bool hit(Ray& ray, HitPoint& hitPoint) const;
};

