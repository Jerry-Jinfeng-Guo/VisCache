#include "precomp.h"
#include "Material.h"

// Constructor, initialize a grey diffuse material
Material::Material()
: type(DIFFUSE), color(RGBColor(0.5f)), isLight(false), isChecker(false), emission(RGBColor(0.0f))
{
}
// Construct from a matType and Color
Material::Material(matType t, RGBColor& c)
: type(t), color(c), isLight(false), isChecker(false), emission(RGBColor(0.0f))
{
}
// Construct from Type, Color and Emmision
Material::Material(matType t, RGBColor& c, RGBColor& e)
: type(t), color(c), isChecker(false), emission(e)
{
	if (e.r != 0.0f || e.g != 0.0f || e.b != 0.0f || t == GLASS)
		isLight = true;
}
// Destructor
Material::~Material()
{
}
// Copy Constructor
Material::Material(const Material& mat)
: type(mat.type), color(mat.color), isChecker(mat.isChecker), emission(mat.emission)
{
}
// Assignment Operator
Material& Material::operator=(const Material& mat)
{
	if (this == &mat)
		return *this;
	this->type = mat.type;
	this->color = mat.color;
	this->isLight = mat.isLight;
	this->isChecker = mat.isChecker;
	this->emission = mat.emission;
	return *this;
}
// Set Color
void Material::set_color(RGBColor& c)
{
	this->color = c;
}
// Set Material Texture
void Material::set_texture(Texture* tex)
{
	this->texture = tex;
}
// Get Color
RGBColor Material::get_color() const
{
	return this->color;
}
// Get Color from texture
RGBColor Material::get_color(float u, float v, const Point3D& p3d) const
{
	return this->texture->value(u, v, p3d);
}
// Get Emission
RGBColor Material::get_emission() const
{
	return this->emission;
}
// Set type
void Material::set_type(matType& t)
{
	this->type = t;
}
// Get Type
matType Material::get_type() const
{
	return this->type;
}
// Add a GeoPrimitive Object to the output list of current Material
void Material::link_to_mat(GeoPrimitive* objPtr)
{
	this->output.push_back(objPtr);
}

/// Lambert Stuff					<<-- Physically Correct
// The BRDF returns the probablity of given directions and location
float Lambert::BRDF(const Ray& ray_in,
	const HitPoint& hitPoint,
	const Ray& ray_out) const
{
	// Both physically plausible (correct in fact)
#if 1
	// More anvanced way, BRDF is cosine/Pi								<--- Visually darker
	float cosine = hitPoint.normal * ray_out.d;
	cosine = cosine < 0.0 ? 0.0f : cosine;
	return cosine * invPI;
#else
	// A pure uniform distribution over the hemisphere, BRDF is 1/Pi.	<--- Visually better
	return invPI;
#endif
}
// scatter returns a direction according to Material type
bool Lambert::scatter(const Ray& ray_in,
	const HitPoint& hitPoint,
	Ray& ray_out,
	RGBColor& alb,
	float& pdf_value) const
{
	//// Old way of sampling hemi-sphere
	//Point3D samplePoint = hitPoint.point + hitPoint.normal + random_in_unit_sphere();
	//Vector3D sampleDir = samplePoint - hitPoint.point;	
	//// Uniformly sample the Hemisphere
	ONB ONB_helper;	ONB_helper.build_onb(hitPoint.normal);
	Vector3D sampleDir = ONB_helper.local(random_cosine_direction()); sampleDir.normalize();

	ray_out = Ray(hitPoint.point + EPSILON * sampleDir, sampleDir);
	alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
	pdf_value = hitPoint.normal * ray_out.d * invPI;
	return true;
}

/// Matte stuff						<<-- Physically Incorrect, for visual purpose
// scatter returns a direction according to Material type
bool Matte::scatter(const Ray& ray_in,
	const HitPoint& hitPoint,
	Ray& ray_out,
	RGBColor& alb,
	float& pdf_value) const
{
	alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
	return false;
}

/// Mirror stuff					<<-- Physically Correct
// The BRDF returns the probablity of given directions and location
float Mirror::BRDF(const Ray& ray_in,
	const HitPoint& hitPoint,
	const Ray& ray_out) const
{
	// Basically implemented as a Delta Function
	Vector3D reflectDir = ray_in.d - 2.0f * (ray_in.d * hitPoint.normal) * hitPoint.normal;
	if (ray_out.d.x >= reflectDir.x - EPSILON && ray_out.d.x <= reflectDir.x + EPSILON &&
		ray_out.d.y >= reflectDir.y - EPSILON && ray_out.d.y <= reflectDir.y + EPSILON &&
		ray_out.d.z >= reflectDir.z - EPSILON && ray_out.d.z <= reflectDir.z + EPSILON)
		return 1.0f;
	return 0.0f;
}
// scatter returns a direction according to Material type
bool Mirror::scatter(const Ray& ray_in,
	const HitPoint& hitPoint,
	Ray& ray_out,
	RGBColor& alb,
	float& pdf_value) const
{
	Vector3D reflectDir = ray_in.d - 2.0f * (ray_in.d * hitPoint.normal) * hitPoint.normal;
	ray_out = Ray(hitPoint.point + EPSILON * reflectDir, reflectDir);
	alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
	pdf_value = 1.0f;
	return true;
}

/// Dielectric stuff				<<-- Physically Correct
// scatter returns a direction according to Material type
bool Dielectric::scatter(const Ray& ray_in,
	const HitPoint& hitPoint,
	Ray& ray_out,
	RGBColor& alb,
	float& pdf_value) const
{
	alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
	pdf_value = 1.0f;
	// Use two color to hold reflection and refraction
	float reflectRatio = 0.0f;
	// Get the refracted Ray, judge material by Ray<->Normal angle: Glass2Air if > 0
	Vector3D refractDir = Vector3D();
	Ray refractRay = Ray();
	float hitAngle = (-1.0f * ray_in.d) * hitPoint.normal;
	if (hitAngle >= 0)
	{	// From Air to Material
		// Calculated using formula
		//	T = AIR_GLASS*I+(AIR_GLASS*(-1*I*N) - sqrtf(1-AIR_GLASS2*(1-(-1*I*N)*(-1*I*N))))*N
		refractDir = AIR_GLASS * ray_in.d +
			(AIR_GLASS * hitAngle - sqrtf(1.0f - AIR_GLASS2 * (1.0f - hitAngle * hitAngle)))
			* hitPoint.normal;
		refractRay = Ray(hitPoint.point + EPSILON * refractDir, refractDir);

		// Ratio of Reflect-to-Refract is calculated using Schlick Approximation
		// Fr = R0 + (1-R0)(1-cos)^5, Ft = 1-Fr
		reflectRatio = GLASS_R0 + (1.0f - GLASS_R0) * pow((1.0f - hitAngle), 5.0f);
	}
	else
	{	// From Material to Air
		// Calculated using formula
		//	T = GLASS_AIR*I+(GLASS_AIR*(-1*I*-N) - sqrtf(1-GLASS_AIR2*(1-(-1*I*-N)*(-1*I*-N))))*-N
		Normal normal = -1.0f * hitPoint.normal;
		float cosine = (-1.0f * ray_in.d) * normal;
		refractDir = GLASS_AIR * ray_in.d +
			(GLASS_AIR * cosine - sqrtf(1.0f - GLASS_AIR2 * (1.0f - cosine * cosine))) * normal;
		refractRay = Ray(hitPoint.point + EPSILON * refractDir, refractDir);

		// Ratio of Reflect-to-Refract is calculated using Schlick Approximation
		// Fr = R0 + (1-R0)(1-cos)^5, Ft = 1-Fr
		reflectRatio = GLASS_R0 + (1.0f - GLASS_R0) * pow((1.0f - cosine), 5.0f);
	}

	// Pick randomly whether reflect or transmit
	if (drand48() < reflectRatio)
	{
		// Reflect
		Vector3D reflectDir = ray_in.d - 2.0f * (ray_in.d * hitPoint.normal) * hitPoint.normal;
		ray_out = Ray(hitPoint.point + EPSILON * reflectDir, reflectDir);
	}
	else
	{
		// Refract
		ray_out = refractRay;
	}

	return true;
}

/// Metal stuff						<<-- Physically Incorrect, for visual purpose
// The BRDF returns the probablity of given directions and location
float Metal::BRDF(const Ray& ray_in,
	const HitPoint& hitPoint,
	const Ray& ray_out) const
{
	// Mirror metal will cause troubles downstairs
	if (glossness == 0.0f) return 2.0f;
	// A 'Delta' function with Integral = 1.0f supported
	// The surface of the Spherical Sector is 2*pi*r*h = 2*pi*r*r*(1-cos)
	Vector3D reflectDir = ray_in.d - 2.0f * (ray_in.d * hitPoint.normal) * hitPoint.normal;
	float cosine = ray_out.d * reflectDir;
	float cosineExt = sqrtf(1.0f - glossness*glossness);

	// Within the lobe, give cosine/solidAngleMax (sum up to 1.0f)
	if (cosine >= (cosineExt - EPSILON) && cosine <= (1.0f + EPSILON))
		return 1.0f / (TWO_PI * sqrtf(1 - this->glossness *this->glossness));
	return 0.0f;
}
// scatter returns a direction according to Material type
bool Metal::scatter(const Ray& ray_in,
	const HitPoint& hitPoint,
	Ray& ray_out,
	RGBColor& alb,
	float& pdf_value) const
{
	alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);

	Vector3D reflectDir = ray_in.d - 2.0f * (ray_in.d * hitPoint.normal) * hitPoint.normal;
	Point3D samplePoint = hitPoint.point + reflectDir + this->glossness * random_in_unit_sphere();
	Vector3D sampleDir = samplePoint - hitPoint.point;
	ray_out = Ray(hitPoint.point + EPSILON * sampleDir, sampleDir);

#if 0
	// Easier approximation
	pdf_value = 1.0f / (TWO_PI * this->glossness * this->glossness);
#else
	// Physically correct
	pdf_value = 1.0f / (TWO_PI * sqrtf(1 - this->glossness *this->glossness));
#endif

	return true;
}

/// Shiny stuff						<<-- Physically Incorrect, for visual purpose
// The BRDF returns the probablity of given directions and location
float Shiny::BRDF(const Ray& ray_in,
	const HitPoint& hitPoint,
	const Ray& ray_out) const
{
	// If it is a perfect reflected Ray, then 1.0f
	Vector3D reflectDir = ray_in.d - 2.0f * (ray_in.d * hitPoint.normal) * hitPoint.normal;
	if (ray_out.d.x >= reflectDir.x - EPSILON && ray_out.d.x <= reflectDir.x + EPSILON &&
		ray_out.d.y >= reflectDir.y - EPSILON && ray_out.d.y <= reflectDir.y + EPSILON &&
		ray_out.d.z >= reflectDir.z - EPSILON && ray_out.d.z <= reflectDir.z + EPSILON)
		return 1.0f;

	// Otherwise, it must be a Diffuse sample Ray  <- For physical correctness
	float cosine = hitPoint.normal * ray_out.d;
	cosine = cosine < 0.0 ? 0.0f : cosine;
	return cosine * invPI;
}

// scatter returns a direction according to Material type
bool Shiny::scatter(const Ray& ray_in,
	const HitPoint& hitPoint,
	Ray& ray_out,
	RGBColor& alb,
	float& pdf_value) const
{
	if (drand48() < this->shininess)
	{
		// This part is much like what we do in Mirror
		Vector3D reflectDir = ray_in.d - 2.0f * (ray_in.d * hitPoint.normal) * hitPoint.normal;
		ray_out = Ray(hitPoint.point + EPSILON * reflectDir, reflectDir);
		alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
		pdf_value = 1.0f;
		return true;
	}
	else
	{
		// Switch between the Physically-correct way and the visually better way
#if 0
		// This part is much like what we do in Lambert
		// Next Event Estimation
		if (drand48() < 0.5f)
		{
			//// Old way of sampling hemi-sphere
			//Point3D samplePoint = hitPoint.point + hitPoint.normal + random_in_unit_sphere();
			//Vector3D sampleDir = samplePoint - hitPoint.point;	
			//// Uniformly sample the Hemisphere
			ONB ONB_helper;	ONB_helper.build_onb(hitPoint.normal);
			Vector3D sampleDir = ONB_helper.local(random_cosine_direction()); sampleDir.normalize();

			ray_out = Ray(hitPoint.point + EPSILON * sampleDir, sampleDir);
			alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
			pdf_value = hitPoint.normal * ray_out.d * invPI;
		}
		else
		{
			// Importance Sampling
			std::vector<GeoPrimitive*> lights = this->scene->get_lights();
			int lucky_light = 0;

			// Multiple Importance Sampling if multiple lights
			if (lights.size() != 1)
			{
				std::vector<float> importanceList;
				build_importance_list(lights, hitPoint, importanceList);
				lucky_light = random_light_index(importanceList);
			}

			Point3D samplePoint = lights[lucky_light]->rand_pnt(hitPoint.point); //hitPoint.point
			Vector3D sampleDir = samplePoint - hitPoint.point;
			float distance2 = sampleDir.len_squared();
			sampleDir.normalize();

			float area = lights[lucky_light]->get_area();
			//float cosine = fabs(sampleDir.y);
			float cosine = -1.0f*sampleDir*lights[lucky_light]->get_normal(samplePoint);
			ray_out = Ray(hitPoint.point + EPSILON * sampleDir, sampleDir);
			alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
			pdf_value = distance2 / (area * cosine);
		}
		return true;
#else
		// The visually better way
		alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
		pdf_value = 1.0f;
		return false;
#endif
	}
}

/// Phong stuff						<<-- Physically Correct
// The BRDF returns the probablity of given directions and location
float Phong::BRDF(const Ray& ray_in,
	const HitPoint& hitPoint,
	const Ray& ray_out) const
{
	// The Equation for Modified Phong Model
	// BRDF = kd / Pi + ks * ((a + 2) * cos ^ a) / 2Pi
	// Notice: Above equation will reault in los of exploding values, thus below will use modified one
	Vector3D reflectDir = ray_in.d - 2.0f * (ray_in.d * hitPoint.normal) * hitPoint.normal;
	reflectDir.normalize();

	float cosineS = reflectDir * ray_out.d; cosineS = cosineS < EPSILON ? EPSILON : cosineS;
	float cosine = -1.0f * ray_in.d * ray_out.d; cosine = cosine < 0.0f ? 0.0f : cosine;
#if 0
	float result = kd * invPI + ks * (this->a + 2.0f) * invTWO_PI * pow(cosineS, this->a);
#else
	float result = EPSILON + (this->a + 2.0f) * invTWO_PI * pow(cosineS, this->a);
#endif
	result = result < 0.0f ? 0.0f : result;

	return result;
}
// scatter returns a direction according to Material type
bool Phong::scatter(const Ray& ray_in,
	const HitPoint& hitPoint,
	Ray& ray_out,
	RGBColor& alb,
	float& pdf_value) const
{
	// Notice continue from above Phong::BRDF:
	//	Using the equation will result in lots of exploding values. PDF generated here is extremely small sometimes
#if 0
	if (drand48() < kd)
	{
		// Diffuse
		// Next Event Estimation. No branching here
		if (drand48() < 0.5f)
		{
			//// Old way of sampling hemi-sphere
			//Point3D samplePoint = hitPoint.point + hitPoint.normal + random_in_unit_sphere();
			//Vector3D sampleDir = samplePoint - hitPoint.point;	
			//// Uniformly sample the Hemisphere
			ONB ONB_helper;	ONB_helper.build_onb(hitPoint.normal);
			Vector3D sampleDir = ONB_helper.local(random_phong_direction(this->a)); sampleDir.normalize();

			ray_out = Ray(hitPoint.point + EPSILON * sampleDir, sampleDir);
			alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
			pdf_value = hitPoint.normal * ray_out.d * invPI;// +EPSILON;
		}
		else
		{
			// Importance Sampling
			std::vector<GeoPrimitive*> lights = this->scene->get_lights();
			int lucky_light = 0;

			// Multiple Importance Sampling if multiple lights
			if (lights.size() != 1)
			{
				std::vector<float> importanceList;
				build_importance_list(lights, hitPoint, importanceList);
				lucky_light = random_light_index(importanceList);
			}

			Point3D samplePoint = lights[lucky_light]->rand_pnt(hitPoint.point); //hitPoint.point
			Vector3D sampleDir = samplePoint - hitPoint.point;
			float distance2 = sampleDir.len_squared();
			sampleDir.normalize();

			float area = lights[lucky_light]->get_area();

			float cosine = -1.0f*sampleDir*lights[lucky_light]->get_normal(samplePoint);
			ray_out = Ray(hitPoint.point + EPSILON * sampleDir, sampleDir);
			alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
			pdf_value = distance2 / (area * cosine);
		}
	}
	else
	{
		// Specular
		ONB ONB_helper;	ONB_helper.build_onb(hitPoint.normal);
		Vector3D sampleDir = ONB_helper.local(random_phong_direction(this->a));
		sampleDir.normalize();
		Vector3D reflectDir = ray_in.d - 2.0f * (ray_in.d * hitPoint.normal) * hitPoint.normal;
		reflectDir.normalize();
		float cosine = sampleDir * reflectDir; cosine = cosine < EPSILON ? EPSILON : cosine;
		ray_out = Ray(hitPoint.point + EPSILON * sampleDir, sampleDir);
		alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
		pdf_value = (this->a + 1.0f) * invTWO_PI * pow(cosine, this->a);
		pdf_value = pdf_value < EPSILON ? EPSILON : pdf_value;
	}
#else
	// Only Phong Specular
	ONB ONB_helper;	ONB_helper.build_onb(hitPoint.normal);
	Vector3D sampleDir = ONB_helper.local(random_phong_direction(this->a));
	sampleDir.normalize();
	Vector3D reflectDir = ray_in.d - 2.0f * (ray_in.d * hitPoint.normal) * hitPoint.normal;
	reflectDir.normalize();
	float cosine = sampleDir * reflectDir; cosine = cosine < EPSILON ? EPSILON : cosine;
	ray_out = Ray(hitPoint.point + EPSILON * sampleDir, sampleDir);
	alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
	pdf_value = (this->a + 1.0f) * invTWO_PI * pow(cosine, this->a);
	pdf_value = pdf_value < EPSILON ? EPSILON : pdf_value;
#endif
	return true;
}

/// Microfacet stuff				<<-- Physically Correct
// The BRDF returns the probablity of given directions and location
float Microfacet::BRDF(const Ray& ray_in,
	const HitPoint& hitPoint,
	const Ray& ray_out) const
{
	/// Microfacet is evaluated in Local Space!
	ONB ONB_helper; ONB_helper.build_onb(hitPoint.normal);
	Vector3D N = ONB_helper.de_local(hitPoint.normal); N.normalize();
	Vector3D L = ONB_helper.de_local(ray_out.d); L.normalize();
	Vector3D V = ONB_helper.de_local(-1.0f*ray_in.d); V.normalize();

	Vector3D Half = L + V;	Half.normalize();
	float NH = N * Half, NL = N * L;
	float NV = N * V, invVH = 1.0f / (V * Half);
	NH = NH < 0.0f ? 0.0f : NH;			NL = NL < 0.0f ? 0.0f : NL;
	NV = NV < 0.0f ? 0.0f : NV;			invVH = invVH < 0.0f ? 0.0f : invVH;

	float Geo = min(1.0f, min(2.0f * NH * NV * invVH, 2.0f * NH * NL * invVH));
	float Fr = k + (1.0f - k)* pow((1.0f - Half * L), 5.0f);
	float Dh = invTWO_PI * (a + 2.0f) * pow(Half.z, a);
	float Denom = 4.0f * NV * NL;

	float result = Geo * Fr * Dh / Denom; result = result < 0.0f ? 0.0f : result;
	return result;
}
// scatter returns a direction according to Material type
bool Microfacet::scatter(const Ray& ray_in,
	const HitPoint& hitPoint,
	Ray& ray_out,
	RGBColor& alb,
	float& pdf_value) const
{
	ONB ONB_helper;	ONB_helper.build_onb(hitPoint.normal);

	Vector3D H = random_microfacet_direction(this->a); H.normalize();
	Vector3D N = ONB_helper.de_local(hitPoint.normal); N.normalize();
	Vector3D V = ONB_helper.de_local(-1.0f * ray_in.d); V.normalize();
	float VH = V * H; VH = VH < 0.0f ? 0.0f : VH;
	Vector3D L = 2.0f * VH * H - V;	L.normalize();

	float HN = H*N; HN = HN < 0.0f ? 0.0f : HN;

	alb = this->get_color(hitPoint.u, hitPoint.v, hitPoint.point);
	pdf_value = (this->a + 2.0f) * pow(H.z, (this->a + 1.0f)) / (8.0f * PI * VH);
	L = ONB_helper.local(L);
	ray_out = Ray(hitPoint.point + EPSILON * L, L);
	return true;
}