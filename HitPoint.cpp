#include <math.h>
#include "precomp.h"
#include "HitPoint.h"

// Defult constructor, black color with no lighting
HitPoint::HitPoint()
:	hit(false), point(Point3D()), normal(Normal()), 
	ray_t(VERYBIG), u(0.0f), v(0.0f), color(RGBColor()),
	color_light(RGBColor(0.0f)), intensity_light(0.0f),
	ray_counter(0)
{
}
// Copy constructor
HitPoint::HitPoint(const HitPoint& hitPoint)
:	hit(hitPoint.hit), point(hitPoint.point),
	normal(hitPoint.normal), ray_t(hitPoint.ray_t),
	u(hitPoint.u), v(hitPoint.v), color(hitPoint.color),
	color_light(hitPoint.color_light), intensity_light(hitPoint.intensity_light),
	ray_counter(hitPoint.ray_counter), material(hitPoint.material)
{
}
// Assign operator
HitPoint& HitPoint::operator= (const HitPoint& rhs)
{
	if (this == &rhs)
		return *this;
	this->u					= rhs.u;
	this->v					= rhs.v;
	this->hit				= rhs.hit;
	this->color				= rhs.color;
	this->point				= rhs.point;
	this->ray_t				= rhs.ray_t;
	this->normal			= rhs.normal;
	this->material			= rhs.material;
	this->color_light		= rhs.color_light;
	this->ray_counter		= rhs.ray_counter;
	this->intensity_light	= rhs.intensity_light;

	return *this;
}
// Destructor
HitPoint::~HitPoint()
{
}

// Lighting Resetter
void HitPoint::reset_light()
{
	this->ray_counter = 0;
	this->intensity_light = 0.0f;
	this->color_light = RGBColor(0.0f);
}

// Get the material color according to HitPoint::UV
RGBColor HitPoint::get_color() const
{
	int u_floor = (int)floor(100.0f * this->u);
	int v_floor = (int)floor(100.0f * this->v);

	return ((u_floor + v_floor) % 2 == 0) ? 0.15f * this->material->get_color() :
											1.75f * this->material->get_color();
}