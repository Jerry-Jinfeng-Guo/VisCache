#pragma once

class GeoPrimitive;
class Camera;
class Light;
class BVH;
class EnvBall;
class Material;
class Texture;

class Scene
{
public:
	// Constructor
	Scene(){}
	// Destructor
	~Scene(){}

	// Add geometries
	void add_geo(GeoPrimitive* object);
	// Set Camera
	void set_cam(Camera* camInput);
	// Get Camera
	Camera* get_cam();
	// Set EnvBall
	void set_env_ball(EnvBall* envBall);
	// Get EnvBall
	EnvBall* get_env_ball() const;
	// Add lights
	void add_light(Light* lightInput);
	// Add Materials
	void add_material(Material* mat);
	// Add Textures
	void add_texture(Texture* tex){ this->textures.push_back(tex); }
	// Load *.obj file (triangles only)
	void load_obj(std::string filePath, Material* mat, Point3D& objOrigin);
	// Clear scene
	void clear();
	// Return Objects count
	inline int object_count() { return (int)this->objects.size(); };
	// Return Material count
	inline int material_count(){ return (int)this->materials.size(); }
	inline int light_mat_count(){ return (int)this->light_materials.size(); }
	// Return Texture count
	inline int texture_count(){ return (int)this->textures.size(); }
	// Return Light count
	int light_count();
	// Return light vector
	inline std::vector<GeoPrimitive*> get_lights() { return light_objects; }
	// Return BVH node count
	inline int BVH_node_count() { return this->bvhTree.node_count(); }
	// Build BVH of the scene
	void buildBVH();
	
	/// Build Visibility Cache
	void buildVisCache();
	// The recursive func to trace sample ray inside the scene
	void cache(Ray& ray, int& depth);
	// Render the paraboloid
	void render_paraboloid(const VisCacheCAND& cacheCAND, VisCacheData* cacheData);
	// Get VisCache count, for debug only
	inline int get_cache_count(){ return visCache.m_grid.count; }

	/// Scene-Ray intersection
	bool hit(Ray& ray) const;
	// Scene-Ray intersection, with color
	bool hit(Ray& ray, RGBColor& color) const;
	// Scene-Ray intersection, with hit point
	bool hit(Ray& ray, HitPoint& hitPoint) const;
	// Fast hit check, with bounding boxes
	bool hit2(Ray& ray) const;
	// Fast hit check, with HitPoint
	bool hit2(Ray& ray, HitPoint& hitPoint) const;

	// Get the EnvBall color
	RGBColor get_env_color(const Ray& ray) const;

	// So that Tracer could access members without going public
	// Might not be safe, consider revising later  <<-------------------------
	friend class Tracer;
	friend class Sampler;
	friend class Material;

protected:
	// Containers for all geometries, lignts and Camera pointers
	std::vector<GeoPrimitive*> objects;
	std::vector<GeoPrimitive*> light_objects;
	std::vector<float> static_light_importance;
	// Light is actually only useful for RayTracing
	// As we are using Material.emmission to light things in PathTracing
	std::vector<Light*> lights;
	// A vector of all Materials
	std::vector<Material*> materials;
	std::vector<Material*> light_materials;
	// A vector of all Textures
	std::vector<Texture*> textures;
	Camera* cam;
	BVH bvhTree;
	EnvBall* skySphere;
	// Visibility Caching stuff
	VisCache visCache;
	std::vector<VisCacheData*> visCacheData;
	std::vector<VisCacheCAND> visCacheCand;
};