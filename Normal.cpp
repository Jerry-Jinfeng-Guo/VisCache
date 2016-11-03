#include <math.h>
#include "precomp.h"
#include "Normal.h"

// Defult Constructor
Normal::Normal() : x(0.0f), y(0.0f), z(0.0f)
{
}
// Construct from one value
Normal::Normal(const float value) : x(value), y(value), z(value)
{
}
// Construct from three values
Normal::Normal(const float x_value, const float y_value, const float z_value)
: x(x_value), y(y_value), z(z_value)
{
}
// Copy Constructor, from a Normal
Normal::Normal(const Normal& nrml)
: x(nrml.x), y(nrml.y), z(nrml.z)
{
}
// Copy Constructor, from a Vector3D
Normal::Normal(const Vector3D& vec3)
: x(vec3.x), y(vec3.y), z(vec3.z)
{
}
// Destructor
Normal::~Normal()
{
}

// Assignment Operator, from a Normal
Normal& Normal::operator=(const Normal& nrml)
{
	if (this == &nrml)
		return *this;

	x = nrml.x;
	y = nrml.y;
	z = nrml.z;

	return *this;
}
// Assignment Operator, from a Vector3D
Normal& Normal::operator=(const Vector3D& vec3)
{
	x = vec3.x;
	y = vec3.y;
	z = vec3.z;

	return *this;
}
// Assignment Operator, from a Point3D
Normal& Normal::operator=(const Point3D& p3d)
{
	x = p3d.x;
	y = p3d.y;
	z = p3d.z;

	return *this;
}

// Unary minus
Normal Normal::operator-() const
{
	return Normal(-x, -y, -z);
}
// Addition
Normal Normal::operator+ (const Normal& nrml) const
{
	return Normal(x + nrml.x, y + nrml.y, z + nrml.z);
}
// Compound Addition
Normal& Normal::operator+= (const Normal& nrml)
{
	x += nrml.x;
	y += nrml.y;
	z += nrml.z;
	return *this;
}
// Dot product with a Vector3D on the right
float Normal::operator* (const Vector3D& vec3) const
{
	return (x*vec3.x + y*vec3.y + z*vec3.z);
}
// Dot product with a Normal on the right
float Normal::operator* (const Normal& nrml) const
{
	return (x*nrml.x + y*nrml.y + z*nrml.z);
}
// Multipilication by a float on the right
Normal Normal::operator* (const float value) const
{
	return Normal(x*value, y*value, z*value);
}
// Normalize the normal to a unit one
void Normal::normalize()
{
	float length = sqrtf(x*x + y*y + z*z);
	float invLength = float(1.0f / length);
	x *= invLength;
	y *= invLength;
	z *= invLength;
}

// Non-member functions
// multiplication by a float on the left
Normal operator*(const float f, const Normal& n)
{
	return (Normal(f * n.x, f * n.y, f * n.z));
}

// addition of a vector on the left to return a vector 
Vector3D operator+ (const Vector3D& v, const Normal& n)
{
	return (Vector3D(v.x + n.x, v.y + n.y, v.z + n.z));
}

// addition of a Point3D on the left to return a Point3D 
Point3D operator+ (const Point3D& p3d, const Normal& n)
{
	return Point3D(n.x + p3d.x, n.y + p3d.y, n.z + p3d.z);
}

// subtraction of a normal from a vector to return a vector
Vector3D operator- (const Vector3D& v, const Normal& n)
{
	return (Vector3D(v.x - n.x, v.y - n.y, v.z - n.z));
}

// dot product of a Vector3D on the left and a Normal on the right
float operator* (const Vector3D& v, const Normal& n)
{
	return (v.x * n.x + v.y * n.y + v.z * n.z);
}

// dot product of a Ray on the left and a Normal on the right
float operator* (const Ray& ray, const Normal& n)
{
	return (ray.d.x * n.x + ray.d.y * n.y + ray.d.z * n.z);
}

// dot product of a Point3D on the left and a Normal on the right
float operator* (const Point3D& p3d, const Normal& n)
{
	return (p3d.x * n.x + p3d.y * n.y + p3d.z * n.z);
}

// multiplication by a matrix on the left
Normal operator* (const Matrix& mat, const Normal& n)
{
	return (	Normal(	mat.m[0][0] * n.x + mat.m[1][0] * n.y + mat.m[2][0] * n.z,
						mat.m[0][1] * n.x + mat.m[1][1] * n.y + mat.m[2][1] * n.z,
						mat.m[0][2] * n.x + mat.m[1][2] * n.y + mat.m[2][2] * n.z));
}