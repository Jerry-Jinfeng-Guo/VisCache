#include "precomp.h"
#include "Point3D.h"

// Deafult Constructor
Point3D::Point3D() : x(0.0f), y(0.0f), z(0.0f)
{}
// Construct with one value
Point3D::Point3D(const float value) : x(value), y(value), z(value)
{}
// Construct with three values
Point3D::Point3D(	const float x_vlaue,
					const float y_value,
					const float z_value)
					: x(x_vlaue), y(y_value), z(z_value)
{}
// Copy Constructor
Point3D::Point3D(const Point3D& p3d)
: x(p3d.x), y(p3d.y), z(p3d.z)
{}
// Destructor
Point3D::~Point3D()
{}

// Assignment operator
Point3D& Point3D::operator= (const Point3D& p3d)
{
	if (this == &p3d)
		return *this;
	x = p3d.x;
	y = p3d.y;
	z = p3d.z;
	return *this;
}
// Unary minus
Point3D Point3D::operator- ()
{
	return Point3D(-x, -y, -z);
}
// Subtract two Point3D gives a Vector3D
Vector3D Point3D::operator- (const Point3D& p3d) const
{
	return Vector3D(x - p3d.x, y - p3d.y, z - p3d.z);
}
// Add a Point3D gives another Point3D
Point3D Point3D::operator+ (const Point3D& p3d) const
{
	return Point3D(x + p3d.x, y + p3d.y, z + p3d.z);
}
// Subtract a Vector3D from a Point3D gives another Point3D
Point3D Point3D::operator- (const Vector3D& vec3) const
{
	return Point3D(x - vec3.x, y - vec3.y, z - vec3.z);
}
// Multiplication by a float on the right
Point3D Point3D::operator* (const float value) const
{
	return Point3D(x*value, y*value, z*value);
}
// Squared distance between a Point3D and the Point3D
float Point3D::d_squared(const Point3D& p3d) const
{
	return ( (x - p3d.x) * (x - p3d.x) +
			 (y - p3d.y) * (y - p3d.y) +
			 (z - p3d.z) * (z - p3d.z) );
}
// Distance between a Point3D and the Point3D
float Point3D::distance(const Point3D& p3d) const
{
	return sqrtf(d_squared(p3d));
}
// Get coornidate value according to dimension index, 0:=X, 1:=Y, 2:=Z
float Point3D::dim(int d)
{
	switch (d)
	{
	case 0:
		return this->x;
	case 1:
		return this->y;
	case 2:
	default:
		return this->z;
	}
}

// Non-member functions
// Multiplication by a float on the left
Point3D operator* (float value, const Point3D& p3d)
{
	return Point3D(value * p3d.x, value * p3d.y, value * p3d.z);
}
// Multiplication by a Matrix on the left
Point3D operator* (const Matrix& mat, const Point3D& p3d)
{
	return (Point3D(mat.m[0][0] * p3d.x + mat.m[0][1] * p3d.y + mat.m[0][2] * p3d.z + mat.m[0][3],
					mat.m[1][0] * p3d.x + mat.m[1][1] * p3d.y + mat.m[1][2] * p3d.z + mat.m[1][3],
					mat.m[2][0] * p3d.x + mat.m[2][1] * p3d.y + mat.m[2][2] * p3d.z + mat.m[2][3]));
}