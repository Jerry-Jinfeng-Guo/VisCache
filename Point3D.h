#pragma once

class Matrix;
class Vector3D;

class Point3D
{
public:
	union { struct { float x, y, z; }; float cell[3]; };
public:
	// Constructors
	// Deafult Constructor
	Point3D();
	// Construct with one value
	Point3D(const float value);
	// Construct with three values
	Point3D(const float x_vlaue,
			const float y_value,
			const float z_value);
	// Copy Constructor
	Point3D(const Point3D& p3d);

	// Assignment operator
	Point3D& operator= (const Point3D& p3d);
	// Unary minus
	Point3D operator- ();
	// Subtract two Point3D gives a Vector3D
	Vector3D operator- (const Point3D& p3d) const;
	// Add a Point3D gives another Point3D
	Point3D operator+ (const Point3D& p3d) const;
	// Subtract a Vector3D from a Point3D gives another Point3D
	Point3D operator- (const Vector3D& vec3) const;
	// Multiplication by a float on the right
	Point3D operator* (const float value) const;
	// Squared distance between a Point3D and the Point3D
	float d_squared(const Point3D& p3d) const;
	// Distance between a Point3D and the Point3D
	float distance(const Point3D& p3d) const;
	// Get coornidate value according to dimension index, 0:=X, 1:=Y, 2:=Z
	float dim(int d);
	// Destructor
	~Point3D();
};

// Non-member functions
// Multiplication by a float on the left
Point3D operator* (float value, const Point3D& p3d);
// Multiplication by a Matrix on the left
Point3D operator* (const Matrix& mat, const Point3D& p3d);