#pragma once

class Ray;
class Point3D;

class AABB
{
public:
	// Two sets of coordinates
	//union{ struct{ float x0, y0, z0, x1, y1, z1; }; struct{ Point3D min_p, max_p; }; }; // <- this is not working
	float x0, y0, z0, x1, y1, z1;
public:
	// Constructors
	// Defult Constructor, returns a 'inversed' infinitly big box
	AABB();
	// Construct from six values
	AABB(const float x0_value, const float y0_value, const float z0_value,
		 const float x1_value, const float y1_value, const float z1_value);
	// Construct from two Point3D
	AABB(const Point3D p0, const Point3D p1);
	// Copy Constructor
	AABB(const AABB& aabbox);
	// Destructor
	~AABB();

	// Assignment operator
	AABB& operator= (const AABB& aabbox);
	// Compound Addition, extend by an AABB
	AABB& operator+=(const AABB& aabbox);
	// Compound Addition, extend by a Point3D
	AABB& operator+=(const Point3D& p3d);
	// Equality determination
	bool operator==(const AABB& aabbox) const;
	// Inequality determination
	bool operator!=(const AABB& aabbox) const;
	// Ray-AABB hit function
	bool hit(Ray& ray) const;
	// Point3D / Origin of Ray inside AABB function
	bool inside(const Point3D& p3d) const;
	// Get center Point3D
	Point3D centroid() const;
	// Get surface area of the AABB, for SAH-BVH construction
	float area() const;
	//// Shrink the AABB to a very small box
	//void zeros();
	// Get the most wide spread axis, 0:=X, 1:=Y, 2:=Z
	int principleAxis();
	// Get the max and min value in certain dimension, 0:=X, 1:=Y, 2:=Z
	float dimMax(int dim);
	float dimMin(int dim);
};

/// None member functions: Unite input into an AABB
// Two AABB -> One AABB
AABB Union(const AABB& box1, const AABB& box2);
// One AABB + One Point3D -> One AABB
AABB Union(const AABB& box, const Point3D& p3d);
// Two Point3D -> One AABB
AABB Union(const Point3D p3d1, const Point3D p3d2);
// One Point3D + One Vector3D -> One AABB
AABB Union(const Point3D p3d, const Vector3D v3d);