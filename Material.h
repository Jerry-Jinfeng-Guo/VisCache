#pragma once

class Scene;
class RGBColor;
class Texture;
class GeoPrimitive;
class Ray;
class HitPoint;

enum matType
{
	MATTE,
	DIFFUSE,
	SPECULAR,
	MIRROR,
	GLASS,
	GLOSSY,
	EXTENSION
};

class Material
{
protected:
	// Material Type
	matType type;
	// Material Albedo
	RGBColor color;
	// Material Texture
	Texture* texture;
	// Material Emission. Used for objects/materials that act as 'lights'
	RGBColor emission;
	// Objects that are connected to current material. Used for light importance sampling
	std::vector<GeoPrimitive*> output;
	// Scene Pointer
	Scene* scene;
	
public:
	bool isLight;
	bool isChecker;
	// Defult Constructor
	Material();
	// Construct from Type and Color
	Material(matType t, RGBColor& c);
	// Construct from Type, Color and Emmision
	Material(matType t, RGBColor& c, RGBColor& e);
	~Material();
	// Copy Constructor
	Material(const Material& mat);
	// Assign Operator
	Material& operator=(const Material& mat);
	// Set Color
	void set_color(RGBColor& c);
	// Get Color
	RGBColor get_color() const;
	// Get Color from texture
	RGBColor get_color(float u, float v, const Point3D& p3d) const;
	// Get Emission
	RGBColor get_emission() const;
	// Set Type
	void set_type(matType& t);
	// Get Type
	matType get_type() const;
	// Set Material Texture
	void set_texture(Texture* tex);
	// Set current Material to checkerboard
	void set_checker(){ isChecker = true; };
	// Add a GeoPrimitive Object to the output list of current Material
	void link_to_mat(GeoPrimitive* objPtr);

	friend class Scene;
	friend class Sampler;

	/// Virtual Functions to be implemented in specific materials
	// The BRDF returns the probablity of given directions and location
	virtual float BRDF(const Ray& ray_in, const HitPoint& hitPoint, const Ray& ray_out) const { return 1.0f; }
	// scatter returns a direction according to Material type
	virtual bool scatter(const Ray& ray_in, const HitPoint& hitPoint, Ray& ray_out, RGBColor& alb, float& pdf_value) const { return false; }
};

//DIFFUSE	<<-- Physically Correct
class Lambert : public Material
{
public:
	Lambert(){ this->type = DIFFUSE; }
	Lambert(RGBColor& c)
	{
		this->color = c;
		this->type = DIFFUSE;
	}
	Lambert(RGBColor& c, RGBColor& e)
	{
		this->color = c; 
		this->emission = e;
		this->type = DIFFUSE;
		if (e.r != 0.0f || e.g != 0.0f || e.b != 0.0f)
			this->isLight = true;
	}
	~Lambert(){}
	// The BRDF returns the probablity of given directions and location
	virtual float BRDF(const Ray& ray_in, const HitPoint& hitPoint, const Ray& ray_out) const;
	// scatter returns a direction according to Material type
	virtual bool scatter(const Ray& ray_in, const HitPoint& hitPoint, Ray& ray_out, RGBColor& alb, float& pdf_value) const;
};

//MATTE		<<-- Physically Incorrect, for visual purpose
class Matte : public Material
{
public:
	Matte(){ this->type = MATTE; }
	Matte(RGBColor& c)
	{
		this->color = c;
		this->type = MATTE;
	}
	Matte(RGBColor& c, RGBColor e)
	{
		this->color = c;
		this->emission = e;
		this->type = MATTE;
		if (e.r != 0.0f || e.g != 0.0f || e.b != 0.0f)
			this->isLight = true;
	}
	~Matte(){}
	// The BRDF returns the probablity of given directions and location
	virtual float BRDF(const Ray& ray_in, const HitPoint& hitPoint, const Ray& ray_out) const { return 1.0f; };
	// scatter returns a direction according to Material type
	virtual bool scatter(const Ray& ray_in, const HitPoint& hitPoint, Ray& ray_out, RGBColor& alb, float& pdf_value) const;
};

//MIRROR	<<-- Physically Correct
class Mirror : public Material
{
public:
	Mirror(){ this->type = MIRROR; }
	Mirror(RGBColor& c)
	{
		this->color = c;
		this->type = MIRROR;
	}
	// I don't want people to have mirror as lights, because that does not make any sense in real world :(
	~Mirror(){}
	// The BRDF returns the probablity of given directions and location
	virtual float BRDF(const Ray& ray_in, const HitPoint& hitPoint, const Ray& ray_out) const;
	// scatter returns a direction according to Material type
	virtual bool scatter(const Ray& ray_in, const HitPoint& hitPoint, Ray& ray_out, RGBColor& alb, float& pdf_value) const;
};

//GLASS		<<-- Physically Correct
class Dielectric : public Material
{
public:
	Dielectric() { this->type = GLASS; }
	Dielectric(RGBColor& c)
	{
		this->color = c;
		this->type = GLASS;
	}
	~Dielectric(){}
	// The BRDF returns the probablity of given directions and location
	virtual float BRDF(const Ray& ray_in, const HitPoint& hitPoint, const Ray& ray_out) const { return 1.0f; };
	// scatter returns a direction according to Material type
	virtual bool scatter(const Ray& ray_in, const HitPoint& hitPoint, Ray& ray_out, RGBColor& alb, float& pdf_value) const;
};

//GLOSSY	<<-- Physically Incorrect, for visual purpose
class Metal : public Material
{
private:
	float glossness;

public:
	Metal(){ this->type = GLOSSY; this->glossness = 0.35f; }
	Metal(RGBColor& c) 
	{
		this->color = c;
		this->type = GLOSSY;
		this->glossness = 0.25f;
	}
	Metal(RGBColor& c, float g)
	{
		this->color = c;
		this->type = GLOSSY;
		this->glossness = g < 0.0f ? 0.0f : g > 1.0f ? 1.0f : g*g;
	}
	// Just the same reason as Mirror, this material is not going to be light
	~Metal(){}
	// The BRDF returns the probablity of given directions and location
	virtual float BRDF(const Ray& ray_in, const HitPoint& hitPoint, const Ray& ray_out) const;
	// scatter returns a direction according to Material type
	virtual bool scatter(const Ray& ray_in, const HitPoint& hitPoint, Ray& ray_out, RGBColor& alb, float& pdf_value) const;
};

//SPECULAR	<<-- Physically Incorrect, for visual purpose
// Pretty expensive...
class Shiny: public Material
{
private:
	float shininess;

public:
	Shiny(){ shininess = 0.25f; this->type = SPECULAR; }
	Shiny(RGBColor& c)
	{
		this->color = c;
		shininess = 0.25f;
		this->type = SPECULAR;
	}
	Shiny(RGBColor& c, RGBColor& e)
	{
		this->color = c;
		shininess = 0.25f;
		this->emission = e;
		this->type = SPECULAR;
	}
	Shiny(RGBColor& c, RGBColor& e, float s)
	{
		shininess = s;
		this->color = c;
		this->emission = e;
		this->type = SPECULAR;
	}
	~Shiny(){}
	/// Virtual Functions to be implemented in specific materials
	// The BRDF returns the probablity of given directions and location
	virtual float BRDF(const Ray& ray_in, const HitPoint& hitPoint, const Ray& ray_out) const;
	// scatter returns a direction according to Material type
	virtual bool scatter(const Ray& ray_in, const HitPoint& hitPoint, Ray& ray_out, RGBColor& alb, float& pdf_value) const;
};

//EXTENSION's, i.e. advanced BRDF's
// Phong	<<-- Physically Correct
// The Modified Phong! Not ordinary Phong!
class Phong : public Material
{
private:
	float kd, ks, a;
	
public:
	Phong(){ kd = 0.5f; ks = 0.5f; this->type = EXTENSION; }
	Phong(RGBColor& c)
	{
		this->color = c;
		kd = 0.5f;
		ks = 0.5f;
		a = 1.0f;
		this->type = EXTENSION;
	}
	Phong(RGBColor& c, float kd_, float ks_)
	{
		this->color = c;
		kd = kd_; kd = kd <= 0.0f ? 0.05f : kd;
		ks = ks_;
		a = 1.0f;
		this->type = EXTENSION;
	}
	Phong(RGBColor& c, float kd_, float ks_, float a_)
	{
		this->color = c;
		kd = kd_; kd = kd <= 0.0f ? EPSILON : kd;
		ks = ks_;
		a = a_;
		this->type = EXTENSION;
	}
	// Let not make thing super confusing by letting Phong being light :(
	~Phong(){}
	// The BRDF returns the probablity of given directions and location
	virtual float BRDF(const Ray& ray_in, const HitPoint& hitPoint, const Ray& ray_out) const;
	// scatter returns a direction according to Material type
	virtual bool scatter(const Ray& ray_in, const HitPoint& hitPoint, Ray& ray_out, RGBColor& alb, float& pdf_value) const;
};

// Microfacet <<-- Physically Correct
// The Microfacet BRDF
class Microfacet : public Material
{
private:
	float a, k;
public:
	Microfacet(){ a = 1.0f; this->type = EXTENSION; k = 0.9f; }
	Microfacet(RGBColor& c) { a = 1.0f; this->color = c; this->type = EXTENSION; k = 0.9f; }
	Microfacet(RGBColor& c, float a_) { a = a_ > 0.0f ? 0.5f : a_; this->color = c; this->type = EXTENSION; k = 0.9f; }
	Microfacet(RGBColor& c, float a_, float k_)
	{
		a = a_ < 0.0f ? 0.5f : a_ > 50000.0f ? 50000.0f : a_;
		this->color = c;
		this->type = EXTENSION;
		k = k_ < 0.5f ? 0.5f : k_;
	}
	~Microfacet(){}
	// The BRDF returns the probablity of given directions and location
	virtual float BRDF(const Ray& ray_in, const HitPoint& hitPoint, const Ray& ray_out) const;
	// scatter returns a direction according to Material type
	virtual bool scatter(const Ray& ray_in, const HitPoint& hitPoint, Ray& ray_out, RGBColor& alb, float& pdf_value) const;
};
