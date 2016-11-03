#include <math.h>
#include "precomp.h"
#include "Tracer.h"

// Constructors
// Defult Constructor, need a Scene
Tracer::Tracer(Scene* sceneInput)
{
	this->scene = sceneInput;
	this->ray = Ray();
}

// Construct from a Scene* and a Ray&
Tracer::Tracer(Scene* sceneInput, Ray& rayInput)
{
	this->scene = sceneInput;
	this->ray = rayInput;
}

// Destructor
Tracer::~Tracer()
{
	//delete &ray;
}

// Set the tracer to a (new) Ray
void Tracer::set_ray(Ray& rayInput)
{
	this->ray = rayInput;
}

// Trace the Ray in the scene, with defult depth limit
RGBColor Tracer::trace()
{
	return this->trace(this->ray, 0);
}

// Trace the Ray in the scene, with the given depth limit
RGBColor Tracer::trace(Ray& ray_process, const int depth)
{
	// Give a default color of the sky sphere
	RGBColor color = scene->get_env_color(ray_process);
	if (scene->bvhTree.node_count() == 0)
	{
		return color;
	}

	// Fast check, only test with BVHNode.AABB
	if (!scene->bvhTree.hit_check(ray_process))
		return color;
	
	HitPoint hitPoint = HitPoint();

#if 1										//<<-------------------Compare naive BVH Accelerated and brute force----------
	/// The accelerated BVH<->Ray hit test
	scene->bvhTree.hit(ray_process, hitPoint);
#else
	/// The original Ray<->Scene::objects hit part, which is naive and slow
	for (vector<GeoPrimitive *>::size_type i = 0; i < scene->objects.size(); i++)
	{
		if (scene->objects.at(i)->get_AABB().hit(ray_process))
			scene->objects.at(i)->hit(ray_process, hitPoint);
	}
#endif

	/// Debug only, used for detecting intersection counts of rays, result is a Heat map
	/// Only set to zero when render heat map for single ray tracing mode
#if 0
	uint16_t r_count, g_count, b_count;
	if (hitPoint.ray_counter <= 50)
	{
		b_count = hitPoint.ray_counter;
		g_count = 0;
		r_count = 0;
	}
	else if (hitPoint.ray_counter <= 200)
	{
		b_count = 175 > hitPoint.ray_counter ?(150 - hitPoint.ray_counter) : 0;
		g_count = hitPoint.ray_counter - 50;
		r_count = 0;
	}
	else
	{
		b_count = 0;
		//g_count = 500 - hitPoint.ray_counter;
		g_count = 450 > hitPoint.ray_counter ? (450 - hitPoint.ray_counter) : 0;
		r_count = hitPoint.ray_counter - 200;
	}
	return RGBColor(0.01f * (float)r_count, 0.01f * (float)g_count, 0.005f * (float)b_count);
	
#endif

	// If there is any lights, use them, other wise only material color will be used
	if (scene->lights.size() > 0 && hitPoint.hit)
	{
		this->shade(ray_process, hitPoint, color, depth);
	}
	else if (scene->lights.size() == 0 && hitPoint.hit)	// No light, just use material color
	{
		color = (float)fabs(-1.0f * ray_process.d * hitPoint.normal) *	hitPoint.material->get_color();
	}

	return color;
}

// Shade the color according to HitPoint Info and Ray Info. This is where all materials got shaded, and color is determined
void Tracer::shade(Ray& ray_process, HitPoint& hitPoint, RGBColor& color, const int depth)
{
	hitPoint.reset_light();
	if (hitPoint.material->get_type() == DIFFUSE)			//<<----------------------------------------------------------
	{	// Diffuse objects behave according to cosine law
		for (vector<Light*>::size_type j = 0; j < scene->lights.size(); j++)
		{
			Vector3D direction = scene->lights.at(j)->get_position() - hitPoint.point;
			float ray_t = direction.length(); direction.normalize();

			// Move the shadow ray start point a bit along its direction
			Point3D startPoint = hitPoint.point + EPSILON * direction;
			Ray shadowRay(startPoint, direction);
			shadowRay.t = ray_t;

			if (shadowRay * hitPoint.normal > 0.0f)
			{
				//Check hitPoint<->Light visibility
				if (!scene->bvhTree.hit(shadowRay))
				{
					hitPoint.intensity_light += shadowRay * hitPoint.normal
												* scene->lights.at(j)->get_fullIntensity();
					hitPoint.color_light += shadowRay * hitPoint.normal * scene->lights.at(j)->get_fullIntensity()
											* scene->lights.at(j)->get_color();
				}
			}
		}
		// If the material of current hit point is a checker board, then use HitPoint::get_color()
		RGBColor materialColor = hitPoint.material->isChecker ? hitPoint.get_color() : hitPoint.material->get_color();
		
		color = (float)(hitPoint.intensity_light + AMBIENT) *
				fabs(-1.0f * ray_process.d * hitPoint.normal) *
				(materialColor + hitPoint.color_light);
	}
	else if (hitPoint.material->get_type() == MIRROR)		//<<----------------------------------------------------------
	{	// Mirror objects have pure reflection from environment, self-color is black, i.e. does not contribute
		// Get the reflected Ray
		color = hitPoint.material->get_color();
		// If deeper than RAYDEPTH, break recursion with current calculated RGBColor
		if (depth >= RAYDEPTH)
			return;

		Vector3D reflectDir = ray_process.d - 2.0f * (ray_process.d * hitPoint.normal) * hitPoint.normal;
		// Generate reflect ray, notice 20.0f is used to compensate for the white dots happens around the mirror
		Ray reflectRay(hitPoint.point + 20.0f * EPSILON * reflectDir, reflectDir);

		color = color * this->trace(reflectRay, (depth + 1));
	}
	else if (hitPoint.material->get_type() == MATTE)		//<<----------------------------------------------------------
	{
		color = hitPoint.material->get_color();
	}
	else if (hitPoint.material->get_type() == SPECULAR)		//<<----------------------------------------------------------
	{
		// Specular objects have shinny reflective surfaces, with colors from themselves
		for (vector<Light*>::size_type j = 0; j < scene->lights.size(); j++)
		{
			Vector3D direction = scene->lights.at(j)->get_position() - hitPoint.point;
			float ray_t = direction.length(); direction.normalize();

			// Move the shadow ray start point a bit along its direction
			Point3D startPoint = hitPoint.point + EPSILON * direction;
			Ray shadowRay(startPoint, direction);
			shadowRay.t = ray_t;

			if (shadowRay * hitPoint.normal > 0.0f)
			{
				bool quickShadow = !scene->bvhTree.hit_check(shadowRay);
				//Check hitPoint<->Light visibility
				if (quickShadow)
				{
					hitPoint.intensity_light += shadowRay * hitPoint.normal * scene->lights.at(j)->get_fullIntensity();
					hitPoint.color_light += shadowRay * hitPoint.normal * scene->lights.at(j)->get_fullIntensity()
											* scene->lights.at(j)->get_color();
				}
				else if (!scene->bvhTree.hit(shadowRay))
				{
					hitPoint.intensity_light += shadowRay * hitPoint.normal * scene->lights.at(j)->get_fullIntensity();
					hitPoint.color_light += shadowRay * hitPoint.normal * scene->lights.at(j)->get_fullIntensity()
											* scene->lights.at(j)->get_color();
				}
			}
		}
		color = (float)(hitPoint.intensity_light + AMBIENT) *
				fabs(-1.0f * ray_process.d * hitPoint.normal) *
				(hitPoint.material->get_color() + hitPoint.color_light);

		// If deeper than RAYDEPTH, break recursion with current calculated RGBColor
		if (depth > RAYDEPTH)
			return;

		// Get the reflected Ray
		Vector3D reflectDir = ray_process.d - 2.0f * (ray_process.d * hitPoint.normal) * hitPoint.normal;
		Ray reflectRay(hitPoint.point + 10.0f * EPSILON * reflectDir, reflectDir);

		// The ratio between Reflect and Diffuse is fixed as followed
		color = 0.8f * color + 0.5f * this->trace(reflectRay, (depth + 1));
	}
	else if (hitPoint.material->get_type() == GLASS)		//<<----------------------------------------------------------
	{
		// Use two color to hold reflection and refraction
		RGBColor reflectColor = RGBColor();
		RGBColor refractColor = RGBColor();
		float reflectRatio = 0.0f;
		float refractRatio = 0.0f;
		reflectColor = hitPoint.material->get_color();

		// Return material color if deeper than RAYDEPTH
		if (depth > RAYDEPTH)
		{
			color = reflectColor;
			return;
		}

		// Get the reflected Ray and color
		Vector3D reflectDir = ray_process.d - 2.0f * (ray_process.d * hitPoint.normal) * hitPoint.normal;
		Ray reflectRay(hitPoint.point + EPSILON * reflectDir, reflectDir);
		reflectColor = reflectColor * this->trace(reflectRay, (depth + 1));

		// Get the refracted Ray, judge material by Ray<->Normal angle: Glass2Air if > 0
		Vector3D refractDir = Vector3D();
		Ray refractRay = Ray();
		float hitAngle = (-1.0f * ray_process.d) * hitPoint.normal;
		if (hitAngle > 0 || hitAngle == 0)
		{	// From Air to Material
			// Calculated using formula
			//	T = AIR_GLASS*I+(AIR_GLASS*(-1*I*N) - sqrtf(1-AIR_GLASS2*(1-(-1*I*N)*(-1*I*N))))*N
			refractDir = AIR_GLASS * ray_process.d +
						(AIR_GLASS * hitAngle - sqrtf(1.0f - AIR_GLASS2 * (1.0f - hitAngle * hitAngle)))
						* hitPoint.normal;
			refractRay = Ray(hitPoint.point + EPSILON * refractDir, refractDir);

			// Ratio of Reflect-to-Refract is calculated using Schlick Approximation
			// Fr = R0 + (1-R0)(1-cos)^5, Ft = 1-Fr
			reflectRatio = GLASS_R0 + (1.0f - GLASS_R0) * pow((1.0f - hitAngle), 5.0f);
			refractRatio = 1.0f - reflectRatio;
		}
		else
		{	// From Material to Air
			// Calculated using formula
			//	T = GLASS_AIR*I+(GLASS_AIR*(-1*I*-N) - sqrtf(1-GLASS_AIR2*(1-(-1*I*-N)*(-1*I*-N))))*-N
			Normal normal = -1.0f * hitPoint.normal;
			float cosine = (-1.0f * ray_process.d) * normal;
			refractDir = GLASS_AIR * ray_process.d +
						(GLASS_AIR * cosine - sqrtf(1.0f - GLASS_AIR2 * (1.0f - cosine * cosine))) * normal;
			refractRay = Ray(hitPoint.point + EPSILON * refractDir, refractDir);

			// Ratio of Reflect-to-Refract is calculated using Schlick Approximation
			// Fr = R0 + (1-R0)(1-cos)^5, Ft = 1-Fr
			reflectRatio = GLASS_R0 + (1.0f - GLASS_R0) * pow((1.0f - cosine), 5.0f);
			refractRatio = 1.0f - reflectRatio;
		}

		refractColor = hitPoint.material->get_color();
		refractColor = refractColor * this->trace(refractRay, (depth + 1));

		color = reflectRatio * reflectColor + refractRatio * refractColor;
	}
}

// Trace the RayPacket in the scene
void Tracer::trace_packet(RayPacket& packet)
{
	/// Early out 0: no node at all
	if (scene->bvhTree.node_count() == 0)
	{
		return;
	}

	/// RayPacket<->BVH intersection
	scene->bvhTree.hit_packet(packet);
	
	for (uint32_t i = 0; i < RAY_PACKET_SIZE; i++)
	{
#if 0									// << Draw the ray intersection heat map =====================================
		uint16_t r_count, g_count, b_count;
		if (packet.hitPoints[i].ray_counter <= 50)
		{
			b_count = packet.hitPoints[i].ray_counter;
			g_count = 0;
			r_count = 0;
		}
		else if (packet.hitPoints[i].ray_counter <= 200)
		{
			b_count = 175 > packet.hitPoints[i].ray_counter ? (150 - packet.hitPoints[i].ray_counter) : 0;
			g_count = packet.hitPoints[i].ray_counter - 50;
			r_count = 0;
		}
		else
		{
			b_count = 0;
			//g_count = 500 - hitPoint.ray_counter;
			g_count = 450 > packet.hitPoints[i].ray_counter ? (450 - packet.hitPoints[i].ray_counter) : 0;
			r_count = packet.hitPoints[i].ray_counter - 200;
		}
		packet.colors[i] = RGBColor(0.01f * (float)r_count, 0.01f * (float)g_count, 0.005f * (float)b_count);
#else		// << Draw shaded result
		if (scene->lights.size() > 0 && packet.hitPoints[i].hit)
		{
			this->shade(packet.rays[i], packet.hitPoints[i], packet.colors[i], 0);
		}
		else if (scene->lights.size() == 0 && packet.hitPoints[i].hit)
		{
			packet.colors[i] = (float)fabs(-1.0f * packet.rays[i].d * packet.hitPoints[i].normal)
								* packet.hitPoints[i].material->get_color();
		}
#endif
	}
	return;
}