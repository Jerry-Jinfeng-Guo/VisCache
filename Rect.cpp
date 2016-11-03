#include <math.h>
#include "precomp.h"
#include "Rect.h"

// Defult, construct a unit square at the X-Z plane, size 1, pointing +Y
Rect::Rect()
:	GeoPrimitive(),
	v0(0, 0, 0),
	v1(0, 0, 1),
	v2(1, 0, 0),
	n(0, 1, 0)
{
}
// Construct from three Point3D
Rect::Rect(	Point3D p0,
			Point3D p1,
			Point3D p2	)
: GeoPrimitive(),
  v0(p0),
  v1(p1),
  v2(p2)
{
	this->compute_normal();
}
// Copy Constructor
Rect::Rect(const Rect& rect)
:	GeoPrimitive(),
	v0(rect.v0),
	v1(rect.v1),
	v2(rect.v2),
	n(rect.n)
{
}
// Destructor
Rect::~Rect()
{
}

// Assign operator
Rect& Rect::operator= (const Rect& rect)
{
	if (this == &rect)
		return *this;
	v0 = rect.v0;
	v1 = rect.v1;
	v2 = rect.v2;
	n = rect.n;
	return *this;
}
// Normal computer
void Rect::compute_normal()
{
	// Get the normal using cross product
	n = (v1 - v0) ^ (v2 - v0);
	// Then normalize it
	n.normalize();
}
// Set color
void Rect::set_color(const RGBColor& colorInput)
{
	color = colorInput;
}
// Color getter
RGBColor Rect::get_color()
{
	return color;
}
// Get Bounding Box
AABB Rect::get_AABB() const
{
	Point3D v3((v1.x + v2.x - v0.x), (v1.y + v2.y - v0.y), (v1.z + v2.z - v0.z));
	float delta = 0.0000001;
	return AABB(min(min(min(v0.x, v1.x), v2.x), v3.x) - delta,
				min(min(min(v0.y, v1.y), v2.y), v3.y) - delta,
				min(min(min(v0.z, v1.z), v2.z), v3.z) - delta,
				max(max(max(v0.x, v1.x), v2.x), v3.x) + delta,
				max(max(max(v0.y, v1.y), v2.y), v3.y) + delta,
				max(max(max(v0.z, v1.z), v2.z), v3.z) + delta);
}
// Get Rect area
float Rect::get_area() const
{
	// Just in case the Rect is not initialized as an orthonormal Quad, use twice the area of a Triangle
	Vector3D Va = v1 - v0, Vb = v2 - v0, Vc = v2 - v1;
	float a = Va.length(), b = Vb.length(), c = Vc.length();
	float s = (a + b + c) * 0.5f;
	return 2.0f * sqrtf(s*(s - a)*(s - b)* (s - c));
}
// Generate a random Point3D on surface
Point3D Rect::rand_pnt(const Point3D& p3d)
{
	return this->get_uv(Point2D(drand48(), drand48()));
}
// Return a Point3D corresponding to the Point2D Barycentric coordinates
Point3D Rect::get_uv(const Point2D& p2d)
{
	// If the input p2d is not a valid (u, v), return (0,0,0) //<--- Or neg-INF?
	if (p2d.x < 0 || p2d.y < 0 || p2d.x > 1 || p2d.y > 1)
		return Point3D(0.0);
	return Point3D((1.0f - p2d.x - p2d.y) * v0 + p2d.x * v1 + p2d.y * v2);
}
// Ray-Rect hit function
bool Rect::hit(Ray& ray) const
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

	if (beta> 1.0f || gamma > 1.0f)
		return false;

	float e3 = a * p - b * r + d * s;
	float t_ = e3 * inv_denom;

	if (t_ > EPSILON && t_ < ray.t)
	{
		ray.t = t_;
		//ray.counter++;
		return true;
	}

	return false;
}
// Calculate the UV of the HitPoint
void Rect::cal_uv(HitPoint& hitPoint) const
{
	Vector3D vec01 = v1 - v0;
	Vector3D vec02 = v2 - v0;
	Vector3D vec0p = hitPoint.point - v0;

	hitPoint.u = fabs(vec01 * vec0p) / (vec01.len_squared());
	hitPoint.v = fabs(vec02 * vec0p) / (vec02.len_squared());
}

// Ray-Rect hit function, with RGBColor
bool Rect::hit(Ray& ray, RGBColor& colorOutput) const
{
	if (this->hit(ray))
	{
		colorOutput = this->color;
		return true;
	}
	return false;
}
// Ray-Rect hit function, with HitPoint
bool Rect::hit(Ray& ray, HitPoint& hitPoint) const
{
	bool hit = this->hit(ray);
	hitPoint.ray_counter = ray.counter;
	if (hit)
	{
		hitPoint.ray_counter++;
		hitPoint.hit = true;
		hitPoint.color = this->color;
		hitPoint.point = ray.o + ray.t * ray.d;
		hitPoint.ray_t = ray.t;
		hitPoint.normal = this->n;
		this->cal_uv(hitPoint);
		hitPoint.material = this->material;
	}
	return hit;
}