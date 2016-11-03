#pragma once

class GeoPrimitive
{
protected:
	Material* material;
public:
	GeoPrimitive();
	~GeoPrimitive();

	// Pure Virtual functions
	virtual void set_color(const RGBColor& colorInput) = 0;
	// Naive get color from the object
	virtual RGBColor get_color() = 0;
	// Various hit()'s
	virtual bool hit(Ray& ray) const = 0;
	virtual bool hit(Ray& ray, RGBColor& color) const = 0;
	virtual bool hit(Ray& ray, HitPoint& hitPoint) const = 0;
	// Get AABB
	virtual AABB get_AABB() const = 0;
	// Get the equivalent surface area for Importance Sampling
	virtual float get_area() const = 0;
	// Generate a random Point3D on surface, a Point3D input is required, but only used for Sphere
	virtual Point3D rand_pnt(const Point3D& p3d) = 0;
	// Get normal
	virtual Normal get_normal(const Point3D& p3d) = 0;

	// Set Geo-Material
	void set_material(Material* mat);
	// Get Geo-Material
	Material* get_material(){ return material; }
};