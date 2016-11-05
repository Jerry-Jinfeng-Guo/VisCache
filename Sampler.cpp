#include "precomp.h"
#include "Sampler.h"

// Comstruct from a scene ptr
Sampler::Sampler(Scene* sceneInput)
{
	this->scene = sceneInput;
	this->ray = Ray();
}
Sampler::~Sampler()
{
}
// Set Ray
void Sampler::set_ray(Ray& rayInput)
{
	this->ray = rayInput;
}
// Start sampling the scene with defult depth 0
RGBColor Sampler::sample()
{
	return this->sample(this->ray, 0);
}
// The method to recursively sample the scene
RGBColor Sampler::sample(Ray& ray_process, const int depth)
{
	// Use the SkyDome or not: things will be lighted slightly by the sky
#if 1
	// Give a default color of the sky dome
	RGBColor color = 0.65f * scene->get_env_color(ray_process);
#else
	// Give a default grey-ish color
	RGBColor color = RGBColor(0.5f);
#endif
	// Fast check
	if (scene->bvhTree.node_count() == 0 || !scene->bvhTree.hit_check(ray_process))
		return color;

	// Now into Path Tracing...
	HitPoint hitPoint = HitPoint();
	// Traverse the BVH
	scene->bvhTree.hit(ray_process, hitPoint);
	// Shade the HitPoint, and possibly recursively continue the Random Walk (trace the path)
	if (hitPoint.hit)
	{
		//color.correct_gamma();
		this->shade(ray_process, hitPoint, color, depth);
	}

	return color;
}
// The method to shade current HitPoint, called in sample. Most magic in unidirectional path tracing happens here
void Sampler::shade(Ray& ray_process, HitPoint& hitPoint, RGBColor& color, const int depth)
{
	// Grand preparation
	Ray sampleRay_scater, sampleRay_light;
	RGBColor albedo, contrib_scatter, contrib_light, E;
	float pdf_scatter, pdf_light = 0.0f;
	float weight_scatter = 1.0f, weight_light = 0.0f;
	RGBColor emitted = hitPoint.material->get_emission();
	float cosine = (-1.0f * ray_process.d) * hitPoint.normal;
	cosine = (hitPoint.material->get_type() == MIRROR || hitPoint.material->get_type() == GLASS) ? 1.0f : cosine;
	if (cosine < 0.0f) { emitted = RGBColor(0.0f); cosine = 0.0f; }

	// First scatter the incoming ray
	bool scattered = hitPoint.material->scatter(ray_process, hitPoint, sampleRay_scater, albedo, pdf_scatter);
	// Total absorption, nothing reflected, thus only return material emission + color. Only for Matte.
	if ( !scattered )
	{
		color = emitted + albedo;
		return;
	}

#ifdef NEE
	/// Next Event Estimation - only for diffuse/microfacet surfaces
	if (hitPoint.material->get_type() == EXTENSION ||
		hitPoint.material->get_type() == DIFFUSE)
	{
		// Now sample lights
		// Many-light rendering
		std::vector<GeoPrimitive*> lights = this->scene->get_lights();
		int lucky_light = 0;

		if (lights.size() != 1)
		{
#if 0
			std::vector<float> importanceList;
			// Dynamic way of the importance list
			build_importance_list(lights, hitPoint, importanceList);
			lucky_light = random_light_index(importanceList);
#else
			// Static way of the importance list
			lucky_light = random_light_index(scene->static_light_importance);
#endif
		}

		Point3D samplePoint = lights[lucky_light]->rand_pnt(hitPoint.point); //hitPoint.point
		Vector3D lightSampleDir = samplePoint - hitPoint.point;
		float distance2 = lightSampleDir.len_squared();
		lightSampleDir.normalize();
		
		if (lightSampleDir * lights[lucky_light]->get_normal(samplePoint) <= 0.0f)
		{
			float area = lights[lucky_light]->get_area();
			sampleRay_light = Ray(hitPoint.point + EPSILON * lightSampleDir, lightSampleDir);
			sampleRay_light.t = sqrtf(distance2) - 0.0005f;

			// Visibility test
#ifdef FIRST_BOUNCE
			bool visible = isVisible(sampleRay_light, hitPoint.normal, depth);
#else
			bool visible = isVisible(sampleRay_light, hitPoint.normal);
#endif

			//pdf_light = distance2 / (area * cosine_light);
			pdf_light = distance2 / area; float inv_pdf_light = area / distance2;
			contrib_light = hitPoint.material->BRDF(ray_process, hitPoint, sampleRay_light) *
							lights[lucky_light]->get_material()->get_emission()	*
							visible * inv_pdf_light;
		}
	}
	
	/// Multiple Importance Sampling
	if (!contrib_light.is_black())
	{
#if 0
		// Power heuristic, not necessarily better
		weight_scatter = PowerHeuristic(1, pdf_scatter, 1, pdf_light);
		weight_light = PowerHeuristic(1, pdf_light, 1, pdf_scatter);
#else
		// Balance heuristic
		weight_scatter = BalanceHeuristic(1, pdf_scatter, 1, pdf_light);
		weight_light   = BalanceHeuristic(1, pdf_light, 1, pdf_scatter);
#endif
		E += weight_light * contrib_light * albedo;
	}
#endif
	// For Russian Roulette. Here max color channel is used as probablity, and will be scaled up
	float maxMatColor = albedo.max_channel();
	// Russian Roulette
	if (depth > RAYDEPTH)
	{
		if (drand48() < maxMatColor)
		{
			// Make up for the loss
			albedo /= maxMatColor;
		}
		else
		{
			color = emitted;
			return;
		}
	}

	// Now sample BRDF
	contrib_scatter =	albedo * hitPoint.material->BRDF(ray_process, hitPoint, sampleRay_scater) *
						this->sample(sampleRay_scater, depth + 1) /
						pdf_scatter;
	E += weight_scatter * contrib_scatter;

	// The MC Rendering Equation: 'e + albedo * BRDF * cosine * color / pdf', wherein 'albedo * BRDF * color / pdf' is handled in NEE & MIS 
	color = emitted + E * cosine;
}

// Visibility query, use the cache or directly use the BVH, starting from certain depth
bool Sampler::isVisible(Ray& shadowRay, Normal& norml, const int depth)
{
	// If deeper than 0, i.e. from the second bounce onwards, use the visbility cache
	if (depth > 0)
	{
		return isVisible(shadowRay, norml);
	}

	// If at depth 0, i.e. first bounce, do not use visbility cache, as it introduces errors
	return !this->scene->bvhTree.hit(shadowRay);
}

// Visibility query, use the cache or directly use the BVH
bool Sampler::isVisible(Ray& shadowRay, Normal& norml)
{
#ifdef VIS_CACHE
	int u, v, w;
	this->scene->visCache.XYZ2UVW(shadowRay.o, u, v, w);
	int offset = w * (this->scene->visCache.m_grid.grid_dim[0] * this->scene->visCache.m_grid.grid_dim[1]) + 
				 v * this->scene->visCache.m_grid.grid_dim[0] + u;
	
	// The six neighbours / evils
	int neighbours[6], ul, ur, vl, vr, wl, wr;	
	ul = u - 1 <= 0 ? u : u - 1;
	ur = u + 1 >= this->scene->visCache.m_grid.grid_dim[0] ? this->scene->visCache.m_grid.grid_dim[0] : u + 1;
	vl = v - 1 <= 0 ? v : v - 1;
	vr = v + 1 >= this->scene->visCache.m_grid.grid_dim[1] ? this->scene->visCache.m_grid.grid_dim[1] : v + 1;
	wl = w - 1 <= 0 ? w : w - 1;
	wr = w + 1 >= this->scene->visCache.m_grid.grid_dim[2] ? this->scene->visCache.m_grid.grid_dim[2] : w + 1;

	neighbours[0] = w * (this->scene->visCache.m_grid.grid_dim[0] * this->scene->visCache.m_grid.grid_dim[1]) +
					v * this->scene->visCache.m_grid.grid_dim[0] + ul;
	neighbours[1] = w * (this->scene->visCache.m_grid.grid_dim[0] * this->scene->visCache.m_grid.grid_dim[1]) +
					v * this->scene->visCache.m_grid.grid_dim[0] + ur;
	neighbours[2] = w * (this->scene->visCache.m_grid.grid_dim[0] * this->scene->visCache.m_grid.grid_dim[1]) +
					vl * this->scene->visCache.m_grid.grid_dim[0] + u;
	neighbours[3] = w * (this->scene->visCache.m_grid.grid_dim[0] * this->scene->visCache.m_grid.grid_dim[1]) +
					vr * this->scene->visCache.m_grid.grid_dim[0] + u;
	neighbours[4] = wl * (this->scene->visCache.m_grid.grid_dim[0] * this->scene->visCache.m_grid.grid_dim[1]) +
					v * this->scene->visCache.m_grid.grid_dim[0] + u;
	neighbours[5] = wr * (this->scene->visCache.m_grid.grid_dim[0] * this->scene->visCache.m_grid.grid_dim[1]) +
					v * this->scene->visCache.m_grid.grid_dim[0] + u;

	float visibility = 0.0f;
	if (this->scene->visCache.query(offset, neighbours, shadowRay, norml, visibility))
	{
		// According to cache, visible
		return true;
	}
	else if (visibility == -1.0f)
	{
		// No useful cache in grid, invoke regular BVH traversal
		return !this->scene->bvhTree.hit(shadowRay);
	}
	else
	{
		// According to cache, occuluded
		return false;
	}

#else
	return !this->scene->bvhTree.hit(shadowRay);
#endif
}

// Non-member function
// A Many Light Rendering Algorithm
// The function to randomly select a light index with uniform distribution. Probablity of being chosen equals their importance
int random_light_index(std::vector<float>& listInput)
{
	// listInput should be normalized, i.e. sum up to one
	float accumulate = 0.0f, pivot = drand48();
	int index = 0;
	for (std::vector<float>::size_type i = 0; i < listInput.size(); i++)
	{
		accumulate += listInput[i];
		if (pivot <= accumulate)
		{
			index = (int)i;
			break;
		}	
	}
	return index;
}
// The function to build a importance list for lights
void build_importance_list(	const std::vector<GeoPrimitive*>& lightObjs,
							const HitPoint& hitPoint, 
							std::vector<float>& importanceList	)
{
	float sum = 0.0f;
	for (std::vector<GeoPrimitive*>::size_type i = 0; i < lightObjs.size(); i++)
	{
		Point3D samplePoint = lightObjs[i]->rand_pnt(hitPoint.point);
		Vector3D sampleVecNeg = hitPoint.point - samplePoint;
		float inv_d2 = 1.0f / sampleVecNeg.len_squared(); sampleVecNeg.normalize();
		Normal normal = lightObjs[i]->get_normal(samplePoint);
		float area = lightObjs[i]->get_area();
		float cosine = sampleVecNeg*normal;
		if (cosine < 0.0f) cosine = 0.0f;
		float intensity = lightObjs[i]->get_material()->get_emission().average();
		float importance = intensity * area * cosine * inv_d2;
		importanceList.push_back(importance);
		sum += importance;
	}
	// Normalize
	for (std::vector<float>::size_type i = 0; i < importanceList.size(); i++)
	{
		importanceList[i] = importanceList[i] / sum;
	}
}