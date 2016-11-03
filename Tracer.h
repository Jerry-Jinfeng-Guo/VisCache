#pragma once

class Scene;
class Ray;
class HitPoint;
class RGBColor;
struct RayPacket;

class Tracer
{
private:
	Scene* scene;
	Ray ray;
public:
	// Constructors
	// Defult Constructor, need a Scene
	Tracer(Scene* sceneInput);
	// Construct from a Scene* and a Ray&
	Tracer(Scene* sceneInput, Ray& rayInput);
	~Tracer();
	// Set the tracer to a (new) Ray, for packet traversal
	void set_ray(Ray& rayInput);
	// Trace the Ray in the scene, with defult depth limit
	RGBColor trace();
	// Trace given ray in the scene, with given depth limit
	RGBColor trace(Ray& ray_process, const int depth);
	// Shade the color according to HitPoint Info and Ray Info, where it all happens
	void shade(Ray& ray_process, HitPoint& hitPoint, RGBColor& color, const int depth);
	// Trace the RayPacket in the scene
	void trace_packet(RayPacket& packet);
	// Label to indicate render in shaded mode or in hit-count heat map mode
	int heatMap;
};

