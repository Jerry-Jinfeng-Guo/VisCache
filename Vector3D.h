#pragma once

class Matrix;
class Normal;
class Point3D;

class Vector3D
{
public:
	union { struct { float x, y, z; }; float cell[3]; };
public:
	// Constructors
	// Defult Constructor
	Vector3D();
	// Construct with one float
	Vector3D(float value);
	// Construct with three floats
	Vector3D(float x_value, float y_value, float z_value);
	// Copy Construct from a vector
	Vector3D(const Vector3D& vec3);
	// Construct from a Point3D
	Vector3D(const Point3D& p3d);
	// Construct from a normal
	Vector3D(const Normal& nrml);
	// Destructor
	~Vector3D();

	// Assignment operator, from a Vector3D
	Vector3D& operator=(const Vector3D& vec3);
	// Assignment operator, from a Normal
	Vector3D& operator=(const Normal& nrml);
	// Assignment operator, from a Point3D
	Vector3D& operator=(const Point3D& p3d);

	// Unary minus
	Point3D operator- () const;
	// Get length / norm of the Vector3D
	float length();
	// Get the squared length / norm of the Vector3D
	float len_squared();
	// Multiplication by a float on the right
	Vector3D operator* (const float value) const;
	// Division by a float
	Vector3D operator/ (const float value) const;
	// Addition by another Vector3D
	Vector3D operator+ (const Vector3D& vec3) const;
	// Compound Addition
	Vector3D& operator+= (const Vector3D& vec3);
	// Subtract a Vector3D
	Vector3D operator- (const Vector3D& vec3) const;
	// Dot product
	float operator* (const Vector3D& vec3) const;
	// Cross product
	Vector3D operator^ (const Vector3D& vec3) const;
	// Normalize the Vector3D to a unit Vector3D
	// I used SIMD here, but was slower and introduce error: _mm_rsqrt_ss
	inline void normalize() { float vecLengthInv = 1.0f / sqrtf(x*x+y*y+z*z); x *= vecLengthInv; y *= vecLengthInv; z *= vecLengthInv; }
	// Return a unit Vector3D, and normalize the Vector3D
	Vector3D& hat();
};

// The class Ortho-Normal Basis, used to get a ONB whose Y-axis lines with InputNormal
class ONB
{
public:
	ONB(){}
	~ONB(){}
	Vector3D u_axis, v_axis, w_axis;
	void build_onb(const Normal& n);
	Vector3D local(float a, float b, float c) { return a*u_axis + b*v_axis + c*w_axis; }
	Vector3D local(const Vector3D& vec) { return vec.x*u_axis + vec.y*v_axis + vec.z*w_axis; }
	Vector3D de_local(const Vector3D& vec);
};

// Non-member functions
// Multiplication by a float on the left
Vector3D operator* (const float value, const Vector3D& vec3);
// Multiplication by a matrix on the left
Vector3D operator* (const Matrix& mat, const Vector3D& vec3);
// Addition by a Point3D on the left
inline Point3D operator+ (const Point3D& p3d, const Vector3D& vec3) { return Point3D(vec3.x + p3d.x, vec3.y + p3d.y, vec3.z + p3d.z); }
// Get a Vector3D within a unit sphere
Vector3D random_in_unit_sphere();
// Get a Point2D within a unit disc
Point2D random_in_unit_disc();
// Get a random Vector3D according to cosine(with y-axis) distribution
Vector3D random_cosine_direction();								// Lambert
Vector3D random_cosine_direction(float angleInput);				// For Spherical Sector
Vector3D random_phong_direction(float a);						// Phong
Vector3D random_microfacet_direction(float alpha);				// Microfacet