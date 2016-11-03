#include <math.h>
#include "precomp.h"
#include "Scene.h"

#include "tiny_obj_loader.h"	// 3-rd party Obj importer

// Add geometries
void Scene::add_geo(GeoPrimitive* object)
{
	objects.push_back(object);
	if (object->get_material()->isLight)
	{
		// Put light emitting objects into list, along with their potential importance
		light_objects.push_back(object);
		float light_power = object->get_material()->emission.power();
		float light_area = object->get_area();
		static_light_importance.push_back(light_power*light_area);
	}
}
// Set Camera
void Scene::set_cam(Camera* camInput)
{
	cam = camInput;
}
// Get Camera
Camera* Scene::get_cam()
{
	return cam;
}
// Set EnvBall
void Scene::set_env_ball(EnvBall* envBall)
{
	skySphere = envBall;
}
// Get EnvBall
EnvBall* Scene::get_env_ball() const
{
	return skySphere;
}
// Add lights
void Scene::add_light(Light* lightInput)
{
	lights.push_back(lightInput);
}
// Return Light count
int Scene::light_count()
{
	// First check explicit lights, if so, we are in RayTracing, thus only report them
	int explicitLight = (int)this->lights.size();
	if (explicitLight > 0)
		return explicitLight;
	// Now check objects that are linked to Light emitting materials && in the scene
	int ObjsLinkToLightMaterial = (int)this->light_objects.size();
	return ObjsLinkToLightMaterial;
}
// Add Materials
void Scene::add_material(Material* mat)
{
	this->materials.push_back(mat);
	mat->scene = this;
	if (mat->isLight)
		this->light_materials.push_back(mat);
}
// Load *.obj file (triangles only)
void Scene::load_obj(std::string filePath, Material* mat, Point3D& objOrigin)
{	
	// This method uses TinyObjLoader
	std::string errors;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	bool loaded = tinyobj::LoadObj(shapes, materials, errors, filePath.c_str());

	if (!errors.empty())
	{
		std::cout << "Errors occur while loading Obj file: " << filePath << std::endl;
		std::cerr << errors << std::endl;
		return;
	}

	if (!loaded)
	{
		std::cerr << "Failed to load Obj file: " << filePath << ", abort.\n";
		return;
	}

	std::cout << "Loading Obj file: " << filePath << ": ";
	clock_t startTime = clock();
	uint32_t counter = 0;
	// For all mesh shapes:
	for (vector<tinyobj::shape_t>::size_type m = 0; m < shapes.size(); ++m)
	{
		// For all faces(triangles) in each mesh
		for (uint32_t i = 0; i < shapes[m].mesh.indices.size() / 3; ++i)
		{
			int index = shapes[m].mesh.indices[i * 3];
			Point3D v0 = objOrigin + Point3D(	shapes[m].mesh.positions[3 * index],
												shapes[m].mesh.positions[3 * index + 1],
												shapes[m].mesh.positions[3 * index + 2]);
			index = shapes[m].mesh.indices[i * 3 + 1];
			Point3D v1 = objOrigin + Point3D(	shapes[m].mesh.positions[3 * index],
												shapes[m].mesh.positions[3 * index + 1],
												shapes[m].mesh.positions[3 * index + 2]);
			index = shapes[m].mesh.indices[i * 3 + 2];
			Point3D v2 = objOrigin + Point3D(	shapes[m].mesh.positions[3 * index],
												shapes[m].mesh.positions[3 * index + 1],
												shapes[m].mesh.positions[3 * index + 2]);
			Triangle* triangle = new Triangle(v0, v1, v2);
			triangle->set_material(mat);
			this->add_geo(triangle);

			counter++;
		}
	}
	clock_t finishTime = clock();
	uint32_t duration = finishTime - startTime;
	std::cout	<< counter << " triangles for "
				<< shapes.size() << " mesh(es) loaded in " 
				<< duration << " ms." << std::endl;
}
// Clear scene, never called though
void Scene::clear()
{
	// Clear all stuff, geo, lights and set cam to Null
	objects.clear();
	lights.clear();
	cam = NULL;
}
// Build BVH of the scene
void Scene::buildBVH()
{
	bvhTree = BVH(objects);
	bvhTree.scene = this;
}

// Build Visibility Cache
void Scene::buildVisCache()
{
	clock_t cache_begin = clock();

	/// Create VisCache candidates
	// Create candidates from light
	printf("\nGenerating cache candidates from light..");
	for (int i = 0; i < CACHE_CAND_LIGHT; i++)
	{
		Point3D randomPointOnLight; Normal lightDir;
		int lucky_light = random_light_index(static_light_importance);
		randomPointOnLight = light_objects[lucky_light]->rand_pnt(randomPointOnLight);
		lightDir = light_objects[lucky_light]->get_normal(randomPointOnLight);

		ONB ONB_helper;	ONB_helper.build_onb(lightDir);
		Vector3D sampleDir = ONB_helper.local(random_cosine_direction()); sampleDir.normalize();

		Ray randomRay = Ray(randomPointOnLight + EPSILON * sampleDir, sampleDir);
		int depth = 0;
		cache(randomRay, depth);
	}
	clock_t cache_end = clock();
	float duration = (float)(cache_end - cache_begin);
	int candLight = (int)visCacheCand.size();
	printf(". done. in %f ms, got %d cache candidates\n", duration, candLight);
	// create candidates from camera
	printf("Generating cache candidates from camera..");
	cache_begin = clock();
	for (int i = 0; i < CACHE_CAND_EYE; i++)
	{
		int x = (int)(drand48()*SCRWIDTH);
		int y = (int)(drand48()*SCRHEIGHT);
		Ray randomRay = cam->ray(x, y);
		int depth = 0;
		cache(randomRay, depth);
	}
	cache_end = clock();
	duration = (float)(cache_end - cache_begin);
	int candEye = (int)visCacheCand.size() - candLight;
	printf(". done. in %f ms, got %d cache candidates\n", duration, candEye);

	/// Select from candidates, determine final cache points and build the grid
	/// When inserting each cache point into the grid, render the paraboloid to shadowmap
	
	// Determine cache grid info
	int cand_size = (int)visCacheCand.size();
	// Use the small one
	int clamp_cache_num = cand_size < NUM_VIS_CACHE ? cand_size : NUM_VIS_CACHE;
	visCache.worldBBox = bvhTree.worldBBox();
	int principleAxis = visCache.worldBBox.principleAxis();
	visCache.m_grid.delta = (visCache.worldBBox.dimMax(principleAxis) - visCache.worldBBox.dimMin(principleAxis)) / (VIS_GRID_SIZE-1);
	visCache.m_grid.grid_dim[principleAxis] = VIS_GRID_SIZE;
	// Init counter
	visCache.m_grid.count = 0;
	printf("Selecting %d from all %d candidates.\n", clamp_cache_num, cand_size);
	
	for (int i = 0; i < 3; i++)
	{
		if (i != principleAxis)
		{
			float range = visCache.worldBBox.dimMax(i) - visCache.worldBBox.dimMin(i);
			int dim = (int)(range / visCache.m_grid.delta);
			visCache.m_grid.grid_dim[i] = dim;
		}
	}
	visCache.m_grid.data = (voxel*)MALLOC64(visCache.m_grid.grid_dim[0] * visCache.m_grid.grid_dim[1] * visCache.m_grid.grid_dim[2] * sizeof(voxel));
	memset(visCache.m_grid.data, 0, visCache.m_grid.grid_dim[0] * visCache.m_grid.grid_dim[1] * visCache.m_grid.grid_dim[2] * sizeof(voxel));
	printf("Grid size: %d x %d x %d\n", visCache.m_grid.grid_dim[0], visCache.m_grid.grid_dim[1], visCache.m_grid.grid_dim[2]);

	printf("Computing caches ");
	// Now select from candidates
	int done = 0;
	cache_begin = clock();
	while (done < clamp_cache_num)
	{
		int lucky_cand_id = (int)(drand48()*cand_size);
		lucky_cand_id = lucky_cand_id == cand_size ? cand_size - 1 : lucky_cand_id;
		VisCacheCAND lucky_cand = visCacheCand[lucky_cand_id];
		int u, v, w, offset;
		visCache.XYZ2UVW(lucky_cand.p, u, v, w);
		offset = w * (visCache.m_grid.grid_dim[0] * visCache.m_grid.grid_dim[1]) + v * visCache.m_grid.grid_dim[0] + u;
		
		// Case: no cache in voxel
		if (visCache.m_grid.data[offset].cache_in_voxel == 0)
		{
			// Print progress
			if (done % 400 == 0) printf(".");
			VisCacheData* newCache = new VisCacheData();

			// Render the cache
			render_paraboloid(lucky_cand, newCache);

			visCache.m_grid.data[offset].cache.push_back(newCache);
			visCache.m_grid.data[offset].cache_in_voxel++;
			visCache.m_grid.count++;
		}
		// Case: already cache in voxel
		else if (visCache.m_grid.data[offset].cache_in_voxel < VIS_MAX_PT_VOX)
		{
			bool valuable = true;
			for (int i = 0; i < visCache.m_grid.data[offset].cache_in_voxel; i++)
			{
				// Useless if existing cache have 'similar' Normal
				if (lucky_cand.n * visCache.m_grid.data[offset].cache[i]->n > 0.0f)
				{
					valuable = false;
				}
			}

			if (!valuable)
				continue;

			// Print progress
			if (done % 400 == 0) printf(".");

			VisCacheData* newCache = new VisCacheData();

			// Render the cache
			render_paraboloid(lucky_cand, newCache);

			visCache.m_grid.data[offset].cache.push_back(newCache);
			visCache.m_grid.data[offset].cache_in_voxel++;
			visCache.m_grid.count++;
		}
		// Case: already 6 caches in voxel
		else
		{
			continue;
		}

		done++;
	}
	cache_end = clock();
	duration = (float)(cache_end - cache_begin);
	printf(". done.	in %f ms\n\n", duration);
}
// The recursive function to trace sample ray inside the scene
void Scene::cache(Ray& ray, int& depth)
{
	if (depth >= RAYDEPTH*2) return;
	if (!bvhTree.hit_check(ray)) return;
	HitPoint bouncePoint;	
	if (!bvhTree.hit(ray, bouncePoint)) return;

	// Only cache diffuse and microfacet surfaces
	if (bouncePoint.material->get_type() == DIFFUSE ||
		bouncePoint.material->get_type() == EXTENSION)
	{
		VisCacheCAND candidate;
		candidate.p = bouncePoint.point;
		candidate.n = bouncePoint.normal;
		visCacheCand.push_back(candidate);
		depth++;
	}
	Ray randomRay;
#if 1
	//	Use uniformly distributed random rays
	ONB ONB_helper;	ONB_helper.build_onb(bouncePoint.normal);
	Vector3D sampleDir = ONB_helper.local(random_cosine_direction()); sampleDir.normalize();
	randomRay = Ray(bouncePoint.point + EPSILON * sampleDir, sampleDir);
#else
	//	Use Material::scatter to generate new ray
	RGBColor dummyColor; float dummyFloat;
	bouncePoint.material->scatter(ray, bouncePoint, randomRay, dummyColor, dummyFloat);
#endif

	// Continue the random walk
	cache(randomRay, depth);
}
// Render the paraboloid
void Scene::render_paraboloid(const VisCacheCAND& cacheCAND, VisCacheData* cacheData)
{
	//VisCacheData* cacheData = new VisCacheData;

	cacheData->p = cacheCAND.p; cacheData->n = cacheCAND.n;
	ONB ONB_helper; ONB_helper.build_onb(cacheData->n);

	// Render the paraboloid into the 64x64 shadow map
	for (int j = 0; j < VIS_CACHE_SM_SIZE; j++) for (int i = 0; i < VIS_CACHE_SM_SIZE; i++)
	{
		float x, y, z; 	UV2XYZ(i, j, x, y, z);
		Vector3D dir(x, y, z); dir = ONB_helper.local(dir); dir.normalize();
		Point3D origin = cacheData->p + EPSILON * dir;
		Ray ray(origin, dir);	HitPoint dummyHitPnt;
		if (this->bvhTree.hit_check(ray))
			this->bvhTree.hit(ray, dummyHitPnt);
		cacheData->sm.value[j * VIS_CACHE_SM_SIZE + i] = ray.t;
	}

	//visCacheData.push_back(cacheData);
}

// Scene-Ray intersection
bool Scene::hit(Ray& ray) const
{
	bool state = false;
	for (vector<GeoPrimitive *>::size_type i = 0; i < this->objects.size(); i++)
	{
		if (this->objects.at(i)->get_AABB().hit(ray))
			if (this->objects.at(i)->hit(ray))
				state = true;
	}
	return state;
}
// Scene-Ray intersection, with color
bool Scene::hit(Ray& ray, RGBColor& color) const
{
	bool state = false;
	for (vector<GeoPrimitive *>::size_type i = 0; i < this->objects.size(); i++)
	{
		if (this->objects.at(i)->get_AABB().hit(ray))
			if (this->objects.at(i)->hit(ray, color))
				state = true;
	}
	return state;
}
// Scene-Ray intersection, with hit point
bool Scene::hit(Ray& ray, HitPoint& hitPoint) const
{
	bool state = false;
	for (vector<GeoPrimitive *>::size_type i = 0; i < this->objects.size(); i++)
	{
		if (this->objects.at(i)->get_AABB().hit(ray))
		if (this->objects.at(i)->hit(ray, hitPoint))
		{
			// If there is any lights, use them
			if (this->lights.size() > 0)
			{
				hitPoint.reset_light();
			for (vector<Light*>::size_type j = 0; j < this->lights.size(); j++)
			{
				Vector3D direction = this->lights.at(j)->get_position() - hitPoint.point;
				float ray_t = direction.length(); direction.normalize();

				// Move the shadow ray start point a bit along its direction
				Point3D startPoint = hitPoint.point + EPSILON * direction;
				Ray shadowRay(startPoint, direction);
				shadowRay.t = ray_t;

				if (shadowRay * hitPoint.normal > 0.0f){
				if (!this->hit(shadowRay))
				{
					hitPoint.intensity_light += shadowRay * hitPoint.normal * this->lights.at(j)->get_fullIntensity();
					hitPoint.color_light += shadowRay * hitPoint.normal * this->lights.at(j)->get_fullIntensity() * this->lights.at(j)->get_color();
				}}
			}
			}
			state = true;
		}
	}
	return state;
}

// Fast hit check, with bounding boxes
bool Scene::hit2(Ray& ray) const
{
	for (vector<GeoPrimitive *>::size_type i = 0; i < this->objects.size(); i++)
	{
		if (this->objects.at(i)->get_AABB().hit(ray))
			return true;
	}
	return false;
}

// Scene-Ray intersection, with hit point. This function is for shadowRay-MIRROR intersection
bool Scene::hit2(Ray& ray, HitPoint& hitPoint) const
{
	bool state = false;
	for (vector<GeoPrimitive *>::size_type i = 0; i < this->objects.size(); i++)
	{
		if (this->objects.at(i)->get_AABB().hit(ray))
		if (this->objects.at(i)->hit(ray, hitPoint))
			state = true;
	}
	return state;
}

// Get the EnvBall color
RGBColor Scene::get_env_color(const Ray& ray) const
{
	uint32_t u, v;
	// Map direction to uv coordinates
	u = uint32_t(this->get_env_ball()->get_w() * (0.5f + invTWO_PI * atan2(ray.d.z, ray.d.x)));
	v = uint32_t(this->get_env_ball()->get_h() * (0.5f - invPI * asinf(ray.d.y)));

	return this->get_env_ball()->get_color(u, v);
}