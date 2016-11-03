#include <math.h>
#include "precomp.h"
#include "Vector3D.h"

// Defult Constructor
Vector3D::Vector3D() : x(0.0f), y(0.0f), z(0.0f)
{}
// Construct with one float
Vector3D::Vector3D(float value) : x(value), y(value), z(value)
{}
// Construct with three floats
Vector3D::Vector3D(float x_value, float y_value, float z_value)
: x(x_value), y(y_value), z(z_value)
{}
// Copy Construct from a vector
Vector3D::Vector3D(const Vector3D& vec3)
: x(vec3.x), y(vec3.y), z(vec3.z)
{}
// Construct from a Point3D
Vector3D::Vector3D(const Point3D& p3d)
: x(p3d.x), y(p3d.y), z(p3d.z)
{}
// Construct from a normal
Vector3D::Vector3D(const Normal& nrml)
: x(nrml.x), y(nrml.y), z(nrml.z)
{}
// Destructor
Vector3D::~Vector3D()
{}

// Assignment operator, from a Vector3D
Vector3D& Vector3D::operator=(const Vector3D& vec3)
{
	if (this == &vec3)
		return *this;

	x = vec3.x;
	y = vec3.y;
	z = vec3.z;

	return *this;
}
// Assignment operator, from a Normal
Vector3D& Vector3D::operator=(const Normal& nrml)
{
	x = nrml.x;
	y = nrml.y;
	z = nrml.z;
	return *this;
}
// Assignment operator, from a Point3D
Vector3D& Vector3D::operator=(const Point3D& p3d)
{
	x = p3d.x;
	y = p3d.y;
	z = p3d.z;
	return *this;
}

// Unary minus
Point3D Vector3D::operator- () const
{
	return Point3D(-x, -y, -z);
}
// Get length / norm of the Vector3D
float Vector3D::length()
{
	return sqrtf(len_squared());
}
// Get the squared length / norm of the Vector3D
float Vector3D::len_squared()
{
	return (x*x + y*y + z*z);
}
// Multiplication by a float on the right
Vector3D Vector3D::operator* (const float value) const
{
	return Vector3D(x*value, y*value, z*value);
}
// Division by a float
Vector3D Vector3D::operator/ (const float value) const
{
	float valueInv = float(1.0f / value);
	return Vector3D(x*valueInv, y*valueInv, z*valueInv);
}
// Addition by another Vector3D
Vector3D Vector3D::operator+ (const Vector3D& vec3) const
{
	return Vector3D(x+vec3.x, y+vec3.y, z+vec3.z);
}
// Compound Addition
Vector3D& Vector3D::operator+= (const Vector3D& vec3)
{
	x += vec3.x;
	y += vec3.y;
	z += vec3.z;
	return *this;
}
// Subtract a Vector3D
Vector3D Vector3D::operator- (const Vector3D& vec3) const
{
	return Vector3D(x - vec3.x, y - vec3.y, z - vec3.z);
}
// Dot product
float Vector3D::operator* (const Vector3D& vec3) const
{
	return (x * vec3.x + y * vec3.y + z * vec3.z);
}
// Cross product
Vector3D Vector3D::operator^ (const Vector3D& vec3) const
{
	return (Vector3D(	y * vec3.z - z * vec3.y,
						z * vec3.x - x * vec3.z,
						x * vec3.y - y * vec3.x));
}

// Return a unit Vector3D, and normalize the Vector3D
Vector3D& Vector3D::hat()
{
	float vecLengthInv = float(1.0 / length());
	x *= vecLengthInv;
	y *= vecLengthInv;
	z *= vecLengthInv;

	return *this;
}

/// Class ONB
// Build ONB using input Normal
void ONB::build_onb(const Normal& n)
{
	this->w_axis = n; this->w_axis.normalize();
	Vector3D a = (fabs(n.x) > 0.99f) ? Vector3D(0, 1, 0) : Vector3D(1, 0, 0);
	this->u_axis = this->w_axis^a;
	this->v_axis = this->u_axis^this->w_axis;
	this->u_axis.normalize(); this->v_axis.normalize();
}
Vector3D ONB::de_local(const Vector3D& vec)
{
	return Vector3D(this->u_axis * vec, this->v_axis * vec, this->w_axis * vec);
}

// Non-member functions
// Multiplication by a float on the left
Vector3D operator* (const float value, const Vector3D& vec3)
{
	return Vector3D(vec3.x * value, vec3.y * value, vec3.z * value);
}
// Multiplication by a matrix on the left
Vector3D operator* (const Matrix& mat, const Vector3D& vec3)
{
	return (Vector3D(	mat.m[0][0] * vec3.x + mat.m[0][1] * vec3.y + mat.m[0][2] * vec3.z,
						mat.m[1][0] * vec3.x + mat.m[1][1] * vec3.y + mat.m[1][2] * vec3.z,
						mat.m[2][0] * vec3.x + mat.m[2][1] * vec3.y + mat.m[2][2] * vec3.z));
}

// Get a random Vector3D within a unit sphere
Vector3D random_in_unit_sphere()
{
	Vector3D vec;
	
	do
	{
		vec = 2.0f * Vector3D(drand48(), drand48(), drand48()) - Vector3D(1.0f, 1.0f, 1.0f);
	} while (vec.len_squared() >= 1.0f);
	
	vec.normalize();
	return vec;
}
// Get a Point2D within a unit disc
Point2D random_in_unit_disc()
{
	float x, y;
	do
	{
		x = 2.0f * drand48() - 1.0f; y = 2.0f * drand48() - 1.0f;
	} while (x*x + y*y >= 1.0f);
	return Point2D(x, y);
}
// Get a random Vector3D according to cosine(with z-axis) distribution
// PDF for this distribution is: pdf = cosine / PI
Vector3D random_cosine_direction()
{
	float r1, r2;
	do
	{
		r1 = drand48(); r2 = drand48();
	} while (r1 == 1.0f || r2 == 1.0f);
	float z = (float)sqrtf(1.0f - r2);
	float phi = TWO_PI * r1;
	float rt = (float)sqrtf(r2);
	float x = cosf(phi)*rt;
	float y = sinf(phi)*rt;

	return Vector3D(x, y, z);
}
Vector3D random_cosine_direction(float angleInput)
{
	// This is expensive! That's why there is no sphere lights!
	float theta = (float)acosf(angleInput);
	float r1, r2;
	do
	{
		r1 = drand48(); r2 = drand48();
	} while (r1 == 1.0f || r2 == 1.0f);
	float z = (float)cosf(theta * r2);
	float phi = TWO_PI * r1;
	float sine = (float)sinf(theta * r2);
	float x = cosf(phi)*sine;
	float y = sinf(phi)*sine;

	return Vector3D(x, y, z);
}
Vector3D random_phong_direction(float a)
{
	float r1 = drand48(), r2 = drand48();
	
	float z = pow(r2, 1.0f / (a + 1.0f));
	float t = pow(r2, 2.0f / (a + 1.0f));
	float phi = TWO_PI * r1;
	float rt = (float)sqrtf(1.0f - t);
	float x = cosf(phi)*rt;
	float y = sinf(phi)*rt;

	return Vector3D(x, y, z);
}
Vector3D random_microfacet_direction(float alpha)
{
	float r1, r2;
	do
	{
		r1 = drand48(); r2 = drand48();
	} while (r1 == 0.0f || r2 == 1.0f);
	
	float cosTheta = pow(r1, (1.0f / (alpha + 1)));
	float sinTheta = sqrtf(1.0f - cosTheta*cosTheta);
	float Phi = TWO_PI * r2;

	return Vector3D(	sinTheta * cosf(Phi),
						sinTheta * sinf(Phi),
						cosTheta	);
}
