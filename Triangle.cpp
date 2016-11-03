#include <math.h>
#include "precomp.h"
#include "Triangle.h"

// Defult Constructor, gives a triangle on X-Z plane, Normal +Y
Triangle::Triangle()
:	GeoPrimitive(),
	v0(0, 0, 0),
	v1(0, 0, 1),
	v2(1, 0, 0),
	n (0, 1, 0)
{
}
// Construct from three Point3D
Triangle::Triangle(	const Point3D& p0,
					const Point3D& p1,
					const Point3D& p2	)
:	GeoPrimitive(),
	v0(p0),
	v1(p1),
	v2(p2)
{
	compute_normal();
}
// Copy Constructor
Triangle::Triangle(const Triangle& triangle)
:	GeoPrimitive(),
	v0(triangle.v0),
	v1(triangle.v1),
	v2(triangle.v2),
	n (triangle.n)
{
}
// Destructor
Triangle::~Triangle()
{
}

// Assignment operator
Triangle& Triangle::operator= (const Triangle& triangle)
{
	if (this == &triangle)
		return *this;
	v0 = triangle.v0;
	v1 = triangle.v1;
	v2 = triangle.v2;
	n  = triangle.n;
	return *this;
}
// Normal computer
void Triangle::compute_normal()
{
	// Get the normal using cross product
	n = (v1 - v0) ^ (v2 - v0);
	// Then normalize it
	n.normalize();
}
// Color setter
void Triangle::set_color(const RGBColor& colorInput)
{
	color = colorInput;
}
// Color getter
RGBColor Triangle::get_color()
{
	return color;
}
// Get Bounding Box
AABB Triangle::get_AABB() const
{
	float delta = 0.0000001;
	return AABB(	min(min(v0.x, v1.x), v2.x) - delta,
					min(min(v0.y, v1.y), v2.y) - delta,
					min(min(v0.z, v1.z), v2.z) - delta,
					max(max(v0.x, v1.x), v2.x) + delta,
					max(max(v0.y, v1.y), v2.y) + delta,
					max(max(v0.z, v1.z), v2.z) + delta	);
}
// Get Triangle Area
float Triangle::get_area() const
{
	Vector3D Va = v1 - v0, Vb = v2 - v0, Vc = v2 - v1;
	float a = Va.length(), b = Vb.length(), c = Vc.length();
	float s = (a + b + c) * 0.5f;
	return sqrtf(s*(s - a)*(s - b)* (s - c));
}
// Generate a random Point3D on surface
Point3D Triangle::rand_pnt(const Point3D& p3d)
{
	float x, y;
	do
	{
		x = drand48();
		y = drand48();
	} while (x + y > 1.0f);
	return this->get_uv(Point2D(x, y));
}
// Return a Point3D corresponding to the Point2D Barycentric coordinates
Point3D Triangle::get_uv(const Point2D& p2d)
{
	// If the input p2d is not a valid (u, v), return (0,0,0) //<--- Or neg-INF?
	if (p2d.x < 0 || p2d.y < 0 || p2d.x + p2d.y > 1)
		return Point3D(0.0);
	return Point3D((1.0f - p2d.x - p2d.y) * v0 + p2d.x * v1 + p2d.y * v2);
}
// Ray-Triangle hit function
bool Triangle::hit(Ray& ray) const
{
	ray.counter++;
	float a = v0.x - v1.x, b = v0.x - v2.x, c = ray.d.x, d = v0.x - ray.o.x;
	float e = v0.y - v1.y, f = v0.y - v2.y, g = ray.d.y, h = v0.y - ray.o.y;
	float i = v0.z - v1.z, j = v0.z - v2.z, k = ray.d.z, l = v0.z - ray.o.z;

	float m = f * k - g * j, n = h * k - g * l, p = f * l - h * j;
	float q = g * i - e * k, s = e * j - f * i;

	float inv_denom = 1.0f / (a * m + b * q + c * s);

	float e1 = d * m - b * n - c * p;
	float beta = e1 * inv_denom;

	if (beta < 0.0f)
		return false;

	float r = r = e * l - h * i;
	float e2 = a * n + d * q + c * r;
	float gamma = e2 * inv_denom;

	if (gamma < 0.0f)
		return false;

	if (beta + gamma > 1.0f)
		return false;

	float e3 = a * p - b * r + d * s;
	float t_ = e3 * inv_denom;

	if (t_ > EPSILON && t_ < ray.t)
	{
		ray.t = t_;
		return true;
	}

	return false;
}
// Ray-Triangle hit function, with RGBColor
bool Triangle::hit(Ray& ray, RGBColor& colorOutput) const
{
	if (this->hit(ray))
	{
		colorOutput = this->color;
		return true;
	}
	return false;
}
// Ray-Triangle hit function, with HitPoint
 bool Triangle::hit(Ray& ray, HitPoint& hitPoint) const
{
	 bool hit = this->hit(ray);
	 hitPoint.ray_counter = ray.counter;
	 if (hit)
	 {
		 hitPoint.ray_counter++;
		 hitPoint.hit		= true;
		 hitPoint.color		= this->color;
		 hitPoint.point		= ray.o + ray.t * ray.d;
		 hitPoint.ray_t		= ray.t;
		 hitPoint.normal	= this->n;
		 hitPoint.material	= this->material;
		 return hit;
	 }
	 return hit;
}
// Ray-Triangle hit function, Moller method
bool Triangle::hit_moller(Ray& ray) const
{
	/// This implementation is based on Tomas Moller's method &/ implementation
	/// And it turns out that this method is not the best solution
	float u, v;
	Vector3D E1, E2;
	E1 = v1 - v0;
	E2 = v2 - v0;

	Vector3D Pvec = ray.d ^ E2;
	float det = E1 * Pvec;
	float inv_det = float(1.0f / det);

	Vector3D Tvec = ray.o - v0;
	Vector3D Qvec = Tvec ^ E1;

	if (det > EPSILON)
	{
		u = Tvec * Pvec;
		if (u < 0.0f || u > det)
			return false;

		v = ray.d * Qvec;
		if (v < 0.0f || v > det)
			return false;

		if (u + v > 1.0f)
			return false;
	}
	else if (det < EPSILON)
	{
		u = Tvec * Pvec;
		if (u > 0.0f || u < det)
			return false;

		v = ray.d * Qvec;
		if (v > 0.0f || v < det)
			return false;

		if (u + v > 1.0f)
			return false;
	}
	else
		return false;

	ray.t = inv_det * (E2 * Qvec);
	u *= inv_det;
	v *= inv_det;
	return true;
}