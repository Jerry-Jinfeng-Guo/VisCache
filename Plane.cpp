#include "precomp.h"
#include "Plane.h"

// Defult constructor, gives the x-z plane with gray color
Plane::Plane()
: GeoPrimitive()
{
	p = Point3D(0.0f);
	n = Normal(0.0f, 1.0f, 0.0f);
	color = RGBColor(0.5f);
	d = -1.0f * p * n;
}
// Construct from a Point3D, a Normal and a RGBColor
Plane::Plane(const Point3D& p3d, const Normal& nrml, const RGBColor& colorInput)
: GeoPrimitive(), p(p3d), n(nrml), color(colorInput)
{
	d = -1.0f * p * n;
}
// Copy Constructor
Plane::Plane(const Plane& planeInput)
: GeoPrimitive(), p(planeInput.p), n(planeInput.n), color(planeInput.color), d(planeInput.d)
{
}
// Destructor
Plane::~Plane()
{}

// Assignment operator
Plane& Plane::operator= (const Plane& planeInput)
{
	if (this == &planeInput)
		return *this;
	p = planeInput.p;
	n = planeInput.n;
	color = planeInput.color;
	d = planeInput.d;
	return *this;
}

// Color setter
void Plane::set_color(const RGBColor& colorInput)
{
	color = colorInput;
}
// Color getter
RGBColor Plane::get_color()
{
	return color;
}
// Get AABB, return a INFINITE AABB
AABB Plane::get_AABB() const
{
	return AABB(-1.0f*VERYBIG, -1.0f*VERYBIG, -1.0f*VERYBIG,
				VERYBIG, VERYBIG, VERYBIG);
}
// Generate a random Point3D on surface
Point3D Plane::rand_pnt(const Point3D& p3d)
{
	Point3D rand_p3d;
	do
	{
		rand_p3d.x = drand48();
		rand_p3d.y = drand48();
		rand_p3d.z = drand48();
	} while ((rand_p3d-p) * n != 0.0f);
	return rand_p3d;
}
// Ray-Plane hit function
bool Plane::hit(Ray& ray) const
{
	ray.counter++;
	float t_tmp;
	t_tmp = float(-1.0f * (ray.o * n + d) / (ray.d * n));
	if (t_tmp > EPSILON && t_tmp < ray.t)
	{
		ray.t = t_tmp;
		//ray.counter++;
		return true;
	}
	return false;
}

// Ray-Plane hit function, with color
bool Plane::hit(Ray& ray, RGBColor& colorOut) const
{
	if (this->hit(ray))
	{
		colorOut = this->color;
		return true;
	}
	return false;
}
// Ray-Plane hit function, with hit point
bool Plane::hit(Ray& ray, HitPoint& hitPoint) const
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
		hitPoint.material = this->material;
		return hit;
	}
	return hit;
}