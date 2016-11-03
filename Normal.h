#pragma once

class Matrix;
class Vector3D;
class Point3D;
class Ray;

class Normal
{
public:
	union { struct { float x, y, z; }; float cell[3]; };
public:
	// Constructors
	// Defult Constructor
	Normal();
	// Construct from one value
	inline Normal(const float value);
	// Construct from three values
	inline Normal(const float x_value, const float y_value, const float z_value);
	// Copy Constructor, from a Normal
	Normal(const Normal& nrml);
	// Copy Constructor, from a Vector3D
	Normal(const Vector3D& vec3);
	// Destructor
	inline ~Normal();

	// Assignment Operator, from a Normal
	Normal& operator=(const Normal& nrml);
	// Assignment Operator, from a Vector3D
	Normal& operator=(const Vector3D& vec3);
	// Assignment Operator, from a Point3D
	Normal& operator=(const Point3D& p3d);

	// Unary minus
	Normal operator-() const;
	// Addition
	Normal operator+ (const Normal& nrml) const;
	// Compound Addition
	Normal& operator+= (const Normal& nrml);
	// Dot product with a Vector3D on the right
	float operator* (const Vector3D& vec3) const;
	// Dot product with a Normal on the right
	float operator* (const Normal& nrml) const;
	// Multipilication by a float on the right
	Normal operator* (const float value) const;
	// Normalize the normal to a unit one
	void normalize();
};

// Non-member functions
// multiplication by a float on the left
Normal operator* (const float a, const Normal& n);
// addition of a vector on the left to return a vector 
Vector3D operator+ (const Vector3D& v, const Normal& n);
// addition of a Point3D on the left to return a Point3D 
Point3D operator+ (const Point3D& p3d, const Normal& n);
// subtraction of a normal from a vector to return a vector
Vector3D operator- (const Vector3D&, const Normal& n);
// dot product of a Vector3D on the left and a Normal on the right
float operator* (const Vector3D& v, const Normal& n);
// dot product of a Ray on the left and a Normal on the right
float operator* (const Ray& ray, const Normal& n);
// dot product of a Point3D on the left and a Normal on the right
float operator* (const Point3D& p3d, const Normal& n);
// multiplication by a matrix on the left
Normal operator* (const Matrix& mat, const Normal& n);