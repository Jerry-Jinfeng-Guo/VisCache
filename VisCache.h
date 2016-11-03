#pragma once

class AABB;

struct ShadowMap
{
	float value[VIS_CACHE_SM_SIZE*VIS_CACHE_SM_SIZE];	// at 64*64, 16kb
	//	<TODO>
	//	Possible extension:
	//	add Ambient Occulusion value indicating quality, needs a global threshold
};

struct VisCacheCAND
{
	Point3D p;		// 12
	Normal n;		// 12, 24 bytes. 100k -> 2.3 Kb
};

struct VisCacheData
{
	Point3D p;		// 12
	Normal n;		// 12
	ShadowMap sm;	// at 64*64, ~16 kb. 4k -> ~64 mb
};

struct tinyKdTreeNode
{
	Point3D p;
	VisCacheData* cache;
	tinyKdTreeNode *left, *right;
};

struct voxel
{
	voxel(){ /*cell = AABB();*/ cache_in_voxel = 0; }
	//AABB cell;
	std::vector<VisCacheData*> cache;
	int cache_in_voxel;					// Max 6
};

struct grid
{
	int grid_dim[3];
	float delta;
	int count;
	voxel* data;
	//std::vector<voxel*> data;
	//inline voxel* voxel_at(int u, int v, int w)
	//{
	//	return data[w * (grid_dim[0] * grid_dim[1]) + v * grid_dim[0] + u];
	//}
};

class VisCache
{
public:
	VisCache(){}
	~VisCache(){}
	inline int get_cache_count(){ return m_grid.count; }
	void XYZ2UVW(const Point3D& xyz, int& u, int& v, int& w);
	bool query(int queryPointOffset, int* queryNeighbours, Ray& ray, Normal& nrml, float& result);
	bool query(int offset, Ray& ray, Normal& nrml, float& result);
	// data members
	AABB worldBBox;
	// query structure
	grid m_grid;
};

/// Functions to map spatial dir into shadowmap coordinates
void XYZ2UV(const float &x, const float &y, const float &z, int &u, int &v);
void UV2XYZ(const int &u, const int &v, float &x, float &y, float &z);
inline bool MoreOrLess(float vExact, float vCache) { return ( vExact - 10.0f * EPSILON < vCache ) ? true : false; }