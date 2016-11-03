#pragma once

class Ray;
class Scene;
class HitPoint;
class RGBColor;
class GeoPrimitive;

class Sampler
{
private:
	Scene* scene;
	Ray ray;
public:
	// Comstruct from a scene ptr
	Sampler(Scene* sceneInput);
	~Sampler();
	// Set Ray
	void set_ray(Ray& rayInput);
	// Start sampling the scene with defult depth 0
	RGBColor sample();
	// The method to recursively sample the scene
	RGBColor sample(Ray& ray_process, const int depth);
	// The method to shade current HitPoint, called in sample
	void shade(Ray& ray_process, HitPoint& hitPoint, RGBColor& color, const int depth);
	// Visibility query, use the cache or directly use the BVH
	bool isVisible(Ray& shadowRay, Normal& norml);

	friend class Material;
};

// Non-member function
// Multiple Importance Sampling!
// The function to randomly select a light index with uniform distribution. Probablity of being chosen equals their importance
int random_light_index(std::vector<float>& list);
// The function to build a importance list for lights
void build_importance_list(const std::vector<GeoPrimitive*>&, const HitPoint&, std::vector<float>&);

// Inline non-member functions
// Balanced heuristic and power heuristic, based on Veach's MIS
inline float BalanceHeuristic(int nf, float fPdf, int ng, float gPdf)
{
	return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}

inline float PowerHeuristic(int nf, float fPdf, int ng, float gPdf)
{
	float f = nf * fPdf, g = ng * gPdf;
	return (f * f) / (f * f + g * g);
}
