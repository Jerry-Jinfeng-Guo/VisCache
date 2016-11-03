#include "precomp.h"
#include "AABB.h"

// Defult Constructor
//AABB::AABB() : x0(-1), y0(-1), z0(-1), x1(1), y1(1), z1(1)
AABB::AABB() : x0(VERYBIG), y0(VERYBIG), z0(VERYBIG),
			   x1(-1.0f * VERYBIG), y1(-1.0f * VERYBIG), z1(-1.0f * VERYBIG)
{
}
// Construct from six values
AABB::AABB(	const float x0_value, const float y0_value, const float z0_value,
			const float x1_value, const float y1_value, const float z1_value)
			: x0(x0_value), y0(y0_value), z0(z0_value),
			  x1(x1_value), y1(y1_value), z1(z1_value)
{
}
// Construct from two Point3D
AABB::AABB(const Point3D p0, const Point3D p1)
: x0(p0.x), y0(p0.y), z0(p0.z),
  x1(p1.x), y1(p1.y), z1(p1.z)
{
}
// Copy Constructor
AABB::AABB(const AABB& aabbox)
: x0(aabbox.x0), y0(aabbox.y0), z0(aabbox.z0),
  x1(aabbox.x1), y1(aabbox.y1), z1(aabbox.z1)
{
}
// Destructor
AABB::~AABB()
{
}

// Assignment operator
AABB& AABB::operator= (const AABB& aabbox)
{
	if (this == &aabbox)
		return *this;
	x0 = aabbox.x0; y0 = aabbox.y0; z0 = aabbox.z0;
	x1 = aabbox.x1; y1 = aabbox.y1; z1 = aabbox.z1;
	return *this;
}
// Compound Addition
AABB& AABB::operator+=(const AABB& aabbox)
{
	x0 = min(x0, aabbox.x0); y0 = min(y0, aabbox.y0); z0 = min(z0, aabbox.z0);
	x1 = max(x1, aabbox.x1); y1 = max(y1, aabbox.y1); z1 = max(z1, aabbox.z1);
	return *this;
}
// Equality determination
bool AABB::operator==(const AABB& aabbox) const
{
	return (x0 == aabbox.x0 && y0 == aabbox.y0 && z0 == aabbox.z0 &&
			x1 == aabbox.x1 && y1 == aabbox.y1 && z1 == aabbox.z1);
}
// Inequality determination
bool AABB::operator!=(const AABB& aabbox) const
{
	return (x0 != aabbox.x0 || y0 != aabbox.y0 || z0 != aabbox.z0 ||
			x1 != aabbox.x1 || y1 != aabbox.y1 || z1 != aabbox.z1);
}
// Compound Addition, extend by a Point3D
AABB& AABB::operator+=(const Point3D& p3d)
{
	if (this->inside(p3d))
		return *this;
	x0 = min(x0, p3d.x); y0 = min(y0, p3d.y); z0 = min(z0, p3d.z);
	x1 = max(x1, p3d.x); y1 = max(y1, p3d.y); z1 = max(z1, p3d.z);
	return *this;
}
// Ray-AABB hit function
bool AABB::hit(Ray& ray) const
{
	ray.counter++;
	float ox = ray.o.x; float oy = ray.o.y; float oz = ray.o.z;
	float dx = ray.d.x; float dy = ray.d.y; float dz = ray.d.z;
	
	float tx_min, ty_min, tz_min;
	float tx_max, ty_max, tz_max;

	float a = float(1.0 / dx);
	if (a >= 0) {
		tx_min = (x0 - ox) * a;
		tx_max = (x1 - ox) * a;
	}
	else {
		tx_min = (x1 - ox) * a;
		tx_max = (x0 - ox) * a;
	}

	float b = float(1.0 / dy);
	if (b >= 0) {
		ty_min = (y0 - oy) * b;
		ty_max = (y1 - oy) * b;
	}
	else {
		ty_min = (y1 - oy) * b;
		ty_max = (y0 - oy) * b;
	}

	float c = float(1.0 / dz);
	if (c >= 0) {
		tz_min = (z0 - oz) * c;
		tz_max = (z1 - oz) * c;
	}
	else {
		tz_min = (z1 - oz) * c;
		tz_max = (z0 - oz) * c;
	}

	float t0, t1;

	// find largest entering t value

	if (tx_min > ty_min)
		t0 = tx_min;
	else
		t0 = ty_min;

	if (tz_min > t0)
		t0 = tz_min;

	// find smallest exiting t value

	if (tx_max < ty_max)
		t1 = tx_max;
	else
		t1 = ty_max;

	if (tz_max < t1)
		t1 = tz_max;

	//// EPSILON is a threshould, defined in precomp.h
	//if (t0 < t1 && t1 > EPSILON && t0 < ray.t)
	//{
	//	ray.counter++;
	//	return true;
	//}
	//return false;
	return (t0 < t1 && t1 > EPSILON && t0 < ray.t);
}
// Point3D / Origin of Ray inside AABB function
bool AABB::inside(const Point3D& p3d) const
{
	return ((p3d.x > x0) && (p3d.x < x1) &&
			(p3d.y > y0) && (p3d.y < y1) &&
			(p3d.z > z0) && (p3d.z < z1));
}
// Get center Point3D
Point3D AABB::centroid() const
{
	return Point3D(0.5f * (x0 + x1), 0.5f * (y0 + y1), 0.5f * (z0 + z1));
}
// Get area of the AABB, for SAH-BVH construction
float AABB::area() const
{
	// Note that this returns half the actual area
	return ((x1 - x0) * (y1 - y0) + (x1 - x0) * (z1 - z0) + (y1 - y0) * (z1 - z0));
}
//// Shrink the AABB to a very small box
//void AABB::zeros()
//{
//	x0 = -1.0f * EPSILON;
//	y0 = -1.0f * EPSILON;
//	z0 = -1.0f * EPSILON;
//	x1 = EPSILON;
//	y1 = EPSILON;
//	z1 = EPSILON;
//}

// Get the most wide spread axis, 0:=X, 1:=Y, 2:=Z
int AABB::principleAxis()
{
	float dx = x1 - x0, dy = y1 - y0, dz = z1 - z0;
	if (dx > dy && dx > dz)
		return 0;
	else if (dy > dz)
		return 1;
	else
		return 2;
}
// Get the max and min value in certain dimension, 0:=X, 1:=Y, 2:=Z
float AABB::dimMax(int dim)
{
	switch (dim)
	{
	case 0:
		return x1;
	case 1:
		return y1;
	case 2:
	default:
		return z1;
	}
}
float AABB::dimMin(int dim)
{
	switch (dim)
	{
	case 0:
		return x0;
	case 1:
		return y0;
	case 2:
	default:
		return z0;
	}
}

/// None member functions: Unite input into an AABB
// Two AABB -> One AABB
AABB Union(const AABB& box1, const AABB& box2)
{
	return AABB( min(box1.x0, box2.x0),
				 min(box1.y0, box2.y0),
				 min(box1.z0, box2.z0),
				 max(box1.x1, box2.x1),
				 max(box1.y1, box2.y1),
				 max(box1.z1, box2.z1));
}
// One AABB + One Point3D -> One AABB
AABB Union(const AABB& box, const Point3D& p3d)
{
	return AABB(min(box.x0, p3d.x),
				min(box.y0, p3d.y),
				min(box.z0, p3d.z),
				max(box.x1, p3d.x),
				max(box.y1, p3d.y),
				max(box.z1, p3d.z));
}
// Two Point3D -> One AABB
AABB Union(const Point3D p3d1, const Point3D p3d2)
{
	return AABB(min(p3d1.x, p3d2.x),
				min(p3d1.y, p3d2.y),
				min(p3d1.z, p3d2.z),
				max(p3d1.x, p3d2.x),
				max(p3d1.y, p3d2.y),
				max(p3d1.z, p3d2.z));
}
// One Point3D + One Vector3D -> One AABB
AABB Union(const Point3D p3d, const Vector3D v3d)
{
	return Union(p3d, p3d + v3d);
}