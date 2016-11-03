#include <math.h>
#include "precomp.h"
#include "Sphere.h"

// Defult Constructor, gives a radius 1 grey sphere centered at the orgin
Sphere::Sphere()
: GeoPrimitive(), c(Point3D(0.0)), r(1.0), color(RGBColor(0.5))
{
	r2 = r * r;
}
// Construct from a Point3D, a float radius and a RGBColor
Sphere::Sphere(	const Point3D& center,
				const float& radius, 
				RGBColor& colorInput)
				: GeoPrimitive(), c(center), r(radius), color(colorInput)
{
	r2 = r * r;
}
// Copy Constructor
Sphere::Sphere(const Sphere& sphereInput)
: GeoPrimitive(), c(sphereInput.c), r(sphereInput.r), color(sphereInput.color), r2(sphereInput.r2)
{
}
// Destructor
Sphere::~Sphere()
{
}

// Assignment operator
Sphere& Sphere::operator= (const Sphere& sphereInput)
{
	if (this == &sphereInput)
		return *this;
	c = sphereInput.c;
	r = sphereInput.r;
	r2 = sphereInput.r2;
	color = sphereInput.color;
	return *this;
}
// Color setter
void Sphere::set_color(const RGBColor& colorInput)
{
	color = colorInput;
}
// Get color
RGBColor Sphere::get_color()
{
	return color;
}
// Get Bounding Box
AABB Sphere::get_AABB() const
{
	float delta = 0.0000001;
	return AABB(	c.x - r - delta,
					c.y - r - delta,
					c.z - r - delta,
					c.x + r + delta,
					c.y + r + delta,
					c.z + r + delta	);
}
// Get normal
Normal Sphere::get_normal(const Point3D& p3d)
{
	Vector3D vec = p3d - this->c;
	vec.normalize();
	return Normal(vec);
}
// Generate a random Point3D on surface, the only case where Point3D p3d is used
Point3D Sphere::rand_pnt(const Point3D& p3d)
{
#if 0
	// The cheap way, uses the hemisphere on p3d side
	Vector3D vec;
	do
	{
		vec = random_in_unit_sphere();
	} while (vec*(p3d - this->c) <= 0.0f);
	
	vec.normalize();

#else
	// The expensive way, uses the Spherical Sector that p3d can see
	Vector3D c2p = p3d - this->c; float distance = c2p.length();
	Normal normal(c2p); normal.normalize();
	ONB ONB_helper; ONB_helper.build_onb(normal);
	Vector3D vec = ONB_helper.local(random_cosine_direction(this->r / distance));
	
#endif
	return this->c + this->r * vec;
}
// Ray-Sphere hit function
bool Sphere::hit(Ray& ray) const
{
	ray.counter++;
	float t;
	Vector3D temp = ray.o - this->c;
	float a = ray.d * ray.d;
	float b = 2.0f * temp * ray.d;
	float c = temp * temp - this->r2;
	float delta = b * b - 4.0f * a * c;

	if (delta < 0.0f)
		return(false);
	else
	{
		float e = sqrtf(delta);
		float denom = 2.0f * a;
		t = (-b - e) / denom;    // smaller root

		if (t > EPSILON && t < ray.t)
		{
			ray.t = t;
			//ray.counter++;
			return true;
		}

		t = (-b + e) / denom;    // larger root

		if (t > EPSILON && t < ray.t)
		{
			ray.t = t;
			//ray.counter++;
			return true;
		}
	}
	return false;
}

// Ray-Sphere hit function, another hit method, not yet optimized
bool Sphere::hit2(Ray& ray) const
{
	Vector3D rayToCenter(c.x - ray.o.x, c.y - ray.o.y, c.z - ray.o.z);
	float distanceSquared = rayToCenter * rayToCenter;
	// If the ray origin is outside the Sphere
	if (distanceSquared > r2)
	{
		float t_project = rayToCenter * ray.d;
		Vector3D vec_RayToCenter = rayToCenter - t_project * ray.d;
		float closestDistance = vec_RayToCenter * vec_RayToCenter;
		if (closestDistance > r2)
			return false;
		t_project -= sqrtf(r2 - closestDistance);
		if (t_project < ray.t && t_project > 0)
		{
			ray.t = t_project;
			return true;
		}
		return false;
	}
	// Else the ray origin is inside / on surface of the sphere
	else
	{
		float a = ray.d * ray.d;
		float b = 2.0f * ray.d * (-1.0f * rayToCenter);
		float c = distanceSquared - r2;
		float delta = sqrtf(b * b - 4.0f * a * c);

		float t1 = float((-1.0*b - delta) / (2.0f * a));
		float t2 = float((-1.0*b + delta) / (2.0f * a));

		if (t1 == t2)
		{	
			if (t1 < 0)
				return false;
			else if (t1 < ray.t)
			{
				ray.t = t1;
				return true;
			}
			else
				return false;
		}
		else
		{
			float t_max = t1 > t2 ? t1 : t2;
			float t_min = t1 > t2 ? t2 : t1;

			if (t_max < 0)
			{
				return false;
			}
			else if (t_min > 0 && t_min < ray.t)
			{
				ray.t = t_min;
				return true;
			}
			else if (t_max < ray.t)
			{
				ray.t = t_max;
				return true;
			}
			else
				return false;
		}
	}
}

// Ray-Sphere hit function
bool Sphere::hit(Ray& ray, RGBColor& colorOut) const
{
	if (this->hit(ray))
	{
		colorOut = this->color;
		return true;
	}
	return false;
}

// Ray-Sphere hit function, with hit point
bool Sphere::hit(Ray& ray, HitPoint& hitPoint) const
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
		hitPoint.normal = hitPoint.point - this->c;
		hitPoint.normal.normalize();
		hitPoint.material = this->material;
		hitPoint.u = 0.5f + invTWO_PI * atan2(hitPoint.normal.z, hitPoint.normal.x);
		hitPoint.v = 0.5f - invPI * asinf(hitPoint.normal.y);
		return hit;
	}
	return hit;
}