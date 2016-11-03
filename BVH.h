/*
*	Notice to this file:
*	The basic part of this BVH implementation is inherited mostly from the BVH part in PBRT
*	* this implementation by default uses the binning SAH method
*/
#pragma once

class Point3D;
class Ray;
class AABB;
class GeoPrimitive;
class HitPoint;
class Scene;

struct BVHPrimitiveInfo;
struct BVHBuildNode;
struct LinearBVHNode;
struct RayPacket;

class BVH
{
private:
	// The method to recursively build the normal BVH tree
	BVHBuildNode *recursiveBuild(	std::vector<BVHPrimitiveInfo> &buildData, uint32_t start, uint32_t end,
									uint32_t *totalNodes, std::vector<GeoPrimitive*> &orderedPrims);
	// The method to flatten the normal BVH tree to a linear one
	uint32_t flattenBVHTree(BVHBuildNode *node, uint32_t *offset);

	// Container for all objects
	std::vector<GeoPrimitive*> objects;
	// The final root node of the flatterned BVH
	LinearBVHNode* root;
	// The AABB of the root node
	AABB bbox;
	// Final node number
	int number;

public:
	// Default Constructor, does nothing, but it has to be there
	BVH(){}
	// Construct from objects input
	BVH(const std::vector<GeoPrimitive*> objectsInput);
	// Return the AABB of root node
	AABB worldBBox() const;
	// The Ray<->BVH hit traversal quick check method
	bool hit_check(Ray& ray);
	// The Ray<->BVH hit traversal method
	bool hit(Ray& ray);
	// The Ray<->BVH hit traversal method, with HitPoint
	bool hit(Ray& ray, HitPoint& hitPoint);
	// The RayPacket<->BVH hit traversal method
	bool hit_packet(RayPacket& packet);
	// Get how many nodes in current BVH
	int node_count() const;
	// Destructor
	~BVH();

	// The Scene pointer
	Scene* scene;
};

