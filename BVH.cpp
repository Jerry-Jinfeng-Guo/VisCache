/*
 *	Notice to this file:
 *	The basic part of this BVH implementation is inherited mostly from the BVH part in PBRT
 *	* this implementation by default uses the binning SAH method
 */
#include "precomp.h"
#include "BVH.h"

// This is the primitive information struct, which is mainly used to buffer
// all objects' informaion for normal BVH tree construction 
struct BVHPrimitiveInfo
{
	BVHPrimitiveInfo(){}
	// Construct from an object index in array and the AABB thereof
	BVHPrimitiveInfo(int objIndex, const AABB& BBox)
		: objectIndex(objIndex), bbox(BBox)
	{
		centroid = BBox.centroid();
	}
	int objectIndex;
	Point3D centroid;
	AABB bbox;
};

// Normal BVH node, used for construction, which will later be flatterned
struct BVHBuildNode
{
	BVHBuildNode(){ children[0] = children[1] = NULL; }
	void initLeafNode(uint32_t first, uint32_t n, const AABB& BBox);
	void initInteriorNode(uint32_t axis, BVHBuildNode *child0, BVHBuildNode *child1);
	AABB bbox;
	BVHBuildNode *children[2];
	uint32_t splitAxis, firstPrimOffset, nPrimitives;
};

// Operatorator structs
struct CompareToMid
{
	CompareToMid(int d, float m) { dim = d; mid = m; }
	int dim;
	float mid;
	bool operator()(const BVHPrimitiveInfo &a) const;
};

struct ComparePoints
{
	ComparePoints(int d) { dim = d; }
	int dim;
	bool operator()(const BVHPrimitiveInfo &a,
					const BVHPrimitiveInfo &b) const;
};

struct CompareToBucket
{
	CompareToBucket(int split, int num, int d, const AABB &b)
	: centroidBounds(b), splitBucket(split), nBuckets(num), dim(d){}
	bool operator()(const BVHPrimitiveInfo &p) const;
	int splitBucket, nBuckets, dim;
	const AABB &centroidBounds;
};

// The linear BVH node
struct LinearBVHNode
{
	AABB bbox;
	union
	{
		uint32_t primitivesOffset;    // leaf
		uint32_t secondChildOffset;   // interior
	};

	uint8_t nPrimitives;  // 0 -> interior node
	uint8_t axis;         // interior node: xyz
	uint8_t pad[2];       // ensure 32 byte total size			:) EXCELLENT!
};

bool CompareToMid::operator()(const BVHPrimitiveInfo &a) const
{
	switch (dim)
	{
	case 0:
		return a.centroid.x < mid;
	case 1:
		return a.centroid.y < mid;
	case 2:
		return a.centroid.z < mid;
	default:
		return false;
	}
}

bool ComparePoints::operator()(	const BVHPrimitiveInfo &a,
								const BVHPrimitiveInfo &b) const
{
	switch (dim)
	{
	case 0:
		return a.centroid.x < b.centroid.x;
	case 1:
		return a.centroid.y < b.centroid.y;
	case 2:
		return a.centroid.z < b.centroid.z;
	default:
		return false;
	}
}

bool CompareToBucket::operator()(const BVHPrimitiveInfo &p) const
{
	float numerator = 0.0f, denominator = 0.0f;
	switch (dim)
	{
	case 0:
		numerator = p.centroid.x - centroidBounds.x0;
		denominator = centroidBounds.x1 - centroidBounds.x0;
		break;
	case 1:
		numerator = p.centroid.y - centroidBounds.y0;
		denominator = centroidBounds.y1 - centroidBounds.y0;
		break;
	case 2:	
	default:
		numerator = p.centroid.z - centroidBounds.z0;
		denominator = centroidBounds.z1 - centroidBounds.z0;
		break;
	}
	
	int b = int((float)nBuckets * numerator / denominator);
	if (b == nBuckets)
		b = nBuckets - 1;
	return b <= splitBucket;
}

// The method to initialize the node if it is a leaf node
void BVHBuildNode::initLeafNode(uint32_t first, uint32_t n, const AABB& BBox)
{
	firstPrimOffset = first;
	nPrimitives = n;
	bbox = BBox;
}

// The method to initialize the node if it is an interior node
void BVHBuildNode::initInteriorNode(uint32_t axis, BVHBuildNode *child0, BVHBuildNode *child1)
{
	children[0] = child0;
	children[1] = child1;
	bbox = Union(child0->bbox, child1->bbox);
	splitAxis = axis;
	nPrimitives = 0;
}

// The inline function to test ray<->AABB intersection
static inline bool rayHitBvhAABB(	const AABB &bounds, Ray &ray,
									const Vector3D &invDir, const uint32_t dirIsNeg[3] )
{
	ray.counter++;
	// Check for ray intersection against $x$ and $y$ slabs
	float txmin = (dirIsNeg[0] == 0) ? (bounds.x0 - ray.o.x) : (bounds.x1 - ray.o.x);
	txmin *= invDir.x;
	float txmax = (dirIsNeg[0] == 1) ? (bounds.x0 - ray.o.x) : (bounds.x1 - ray.o.x);
	txmax *= invDir.x;
	float tymin = (dirIsNeg[1] == 0) ? (bounds.y0 - ray.o.y) : (bounds.y1 - ray.o.y);
	tymin *= invDir.y;
	float tymax = (dirIsNeg[1] == 1) ? (bounds.y0 - ray.o.y) : (bounds.y1 - ray.o.y);
	tymax *= invDir.y;

	if ((txmin > tymax) || (tymin > txmax))
		return false;
	if (tymin > txmin)
		txmin = tymin;
	if (tymax < txmax)
		txmax = tymax;

	// Check for ray intersection against $z$ slab
	float tzmin = (dirIsNeg[2] == 0) ? (bounds.z0 - ray.o.z) : (bounds.z1 - ray.o.z);
	tzmin *= invDir.z;
	float tzmax = (dirIsNeg[2] == 1) ? (bounds.z0 - ray.o.z) : (bounds.z1 - ray.o.z);
	tzmax *= invDir.z;
	
	if ((txmin > tzmax) || (tzmin > txmax))
		return false;
	if (tzmin > txmin)
		txmin = tzmin;
	if (tzmax < txmax)
		txmax = tzmax;

	if ((txmin < ray.t) && (txmax > EPSILON) && (txmin < txmax))
	{
		ray.counter++;
		return true;
	}
	return false;
	
	//return (txmin < ray.t) && (txmax > EPSILON) && (txmin < txmax);
}
// The inline function to test frustum<->AABB intersection
static inline bool frustumHitBvhAABB(	const AABB &bounds, const RayPacket& packet	)
{
	Point3D startPoint = packet.rays[packet.frustum[0]].o;

	Vector3D N1 = packet.rays[packet.frustum[1]].d ^ packet.rays[packet.frustum[0]].d;
	Vector3D N2 = packet.rays[packet.frustum[3]].d ^ packet.rays[packet.frustum[1]].d;
	Vector3D N3 = packet.rays[packet.frustum[2]].d ^ packet.rays[packet.frustum[3]].d;
	Vector3D N4 = packet.rays[packet.frustum[0]].d ^ packet.rays[packet.frustum[2]].d;

	Vector3D Va = Point3D(bounds.x0, bounds.y0, bounds.z0) - startPoint;
	Vector3D Vb = Point3D(bounds.x0, bounds.y1, bounds.z0) - startPoint;
	Vector3D Vc = Point3D(bounds.x1, bounds.y0, bounds.z0) - startPoint;
	Vector3D Vd = Point3D(bounds.x0, bounds.y0, bounds.z1) - startPoint;
	Vector3D Ve = Point3D(bounds.x0, bounds.y1, bounds.z1) - startPoint;
	Vector3D Vf = Point3D(bounds.x1, bounds.y0, bounds.z1) - startPoint;

	return	!(	(N1*Va > 0 || N1*Vd > 0) &&
				(N2*Va > 0 || N2*Vd > 0) &&
				(N3*Vb > 0 || N3*Ve > 0) &&
				(N4*Vc > 0 || N4*Vf > 0) );
}

// The inline function to test packet<->AABB intersection
static inline bool packetHitBvhAABB(const AABB &bounds, RayPacket& packet, uint32_t& index)
{
	for (uint32_t i = 0; i < RAY_PACKET_SIZE; ++i)
	{
		Vector3D invDir(1.0f / packet.rays[i].d.x,
						1.0f / packet.rays[i].d.y,
						1.0f / packet.rays[i].d.z);
		uint32_t dirIsNeg[3] = { invDir.x < 0.0f, invDir.y < 0.0f, invDir.z < 0.0f };
		
		if (rayHitBvhAABB(bounds, packet.rays[i], invDir, dirIsNeg))
		{
			index = i;
			return true;
		}
	}
	return false;
}

// Default Constructor, construct from objects input
BVH::BVH(const std::vector<GeoPrimitive*> objectsInput)
{
	// Time counter
	clock_t begin = clock();
	// Return if empty input size
	if (objectsInput.size() == 0)
	{
		root = NULL;
		bbox = AABB();
		number = 0;
		return;
	}

	// Initialize buildData vector for input objects
	objects.clear();
	vector<BVHPrimitiveInfo> buildData;
	buildData.reserve(objectsInput.size());
	for (vector<BVHPrimitiveInfo>::size_type i = 0; i < objectsInput.size(); ++i)
	{
		objects.push_back(objectsInput[i]);
		AABB BBox = objectsInput[i]->get_AABB();
		buildData.push_back(BVHPrimitiveInfo((int)i, BBox));
	}

	// Recursively build BVH tree for objects
	uint32_t totalNodes = 0;
	vector<GeoPrimitive*> orderedObjects;
	orderedObjects.reserve(objects.size());

	BVHBuildNode *buildRoot = recursiveBuild(buildData, 0, (uint32_t)objects.size(), &totalNodes, orderedObjects);

	objects.swap(orderedObjects);
	// Allocate alligned memory for the root ptr to point to
	root = (LinearBVHNode*)malloc(sizeof(LinearBVHNode)* totalNodes);
	// Set each to a new LinearBVHNode*
	for (uint32_t i = 0; i < totalNodes; i++)
	{
		new (&root[i]) LinearBVHNode;
	}
	// Flattern the build tree to a linear tree
	uint32_t offset = 0;
	flattenBVHTree(buildRoot, &offset);
	
	clock_t finish = clock();
	float duration = (float)(finish - begin);

	bbox = root->bbox;
	number = (int)totalNodes;

	// Print BVH construction info:
	std::cout << "\nBVH constructed:\n	" << totalNodes
			  << " nodes for " << (int)objects.size() << " objects\n	using  "
			  << float(totalNodes * sizeof(LinearBVHNode)) / (1024.f*1024.f)
			  << " MB space\n	taking " << duration << " ms." << std::endl;
}

// Destructor
BVH::~BVH()
{}

// Return the AABB of root node
AABB BVH::worldBBox() const
{
	return bbox;
}

// The method to recursively build the normal BVH tree
BVHBuildNode *BVH::recursiveBuild(	std::vector<BVHPrimitiveInfo> &buildData, uint32_t start, uint32_t end,
									uint32_t *totalNodes, std::vector<GeoPrimitive*> &orderedPrims)
{
	// Handle zero-size input
	if (end == start)
		return NULL;
	
	(*totalNodes)++;
	// Allocate a pointer to BVHBuildNode
	BVHBuildNode* node = (BVHBuildNode*)malloc(sizeof(BVHBuildNode));
	
	// Add all objects' AABB into one AABB
	AABB bbox;
	for (uint32_t i = start; i < end; ++i)
	{
		bbox = Union(bbox, buildData[i].bbox);
	}
	uint32_t nPrimitives = end - start;
	float currentCost = (float)nPrimitives * bbox.area();
	// Create current node as a leaf node
	if (nPrimitives == 1)
	{
		// Create leaf node BVHBuildNode
		uint32_t firstPrimOffset = (uint32_t)orderedPrims.size();
		for (uint32_t i = start; i < end; ++i)
		{
			uint32_t primNum = buildData[i].objectIndex;
			orderedPrims.push_back(objects[primNum]);
		}
		node->initLeafNode(firstPrimOffset, nPrimitives, bbox);
	}
	else
	{
		// Current node division and recursive build
		// Compute Centroid Bounds and determine divide dimension
		AABB centroidBounds;
		for (uint32_t i = start; i < end; ++i)
		{
			centroidBounds = Union(centroidBounds, buildData[i].centroid);
		}
		// The dimension to divide
		int dim = centroidBounds.principleAxis();

		// Divide objects into two groups and build children
		uint32_t mid = (uint32_t)((start + end) * 0.5f);
		if (centroidBounds.dimMax(dim) == centroidBounds.dimMin(dim))
		{
			// If nPrimitives is no greater than BVH_MAX_INLEAF,
			// then all the nodes can be stored in a compact bvh leaf node.
			if (nPrimitives <= BVH_MAX_INLEAF)
			{
				// Create leaf _BVHBuildNode_
				uint32_t firstPrimOffset = (uint32_t)orderedPrims.size();
				for (uint32_t i = start; i < end; ++i)
				{
					uint32_t primNum = buildData[i].objectIndex;
					orderedPrims.push_back(objects[primNum]);
				}
				node->initLeafNode(firstPrimOffset, nPrimitives, bbox);
				return node;
			}
			else
			{
				// else if nPrimitives is greater than BVH_MAX_INLEAF, we
				// need to split it further to guarantee each node contains
				// no more than maxPrimsInNode primitives.
				node->initInteriorNode(	dim,
										recursiveBuild(	buildData, start, mid,
														totalNodes, orderedPrims),
										recursiveBuild(	buildData, mid, end,
														totalNodes, orderedPrims));
				return node;
			}
		}

		// The binning-SAH construction, here we use BVH_BINS bins
		// Partition primitives using approximate SAH
		if (nPrimitives <= 4)
		{
			// Partition primitives into equally-sized subsets
			mid = (start + end) / 2;
			std::nth_element(	&buildData[start], &buildData[mid],
								&buildData[end - 1] + 1, ComparePoints(dim));
		}
		else
		{
			// Allocate _BucketInfo_ for SAH partition buckets
			struct BucketInfo
			{
				BucketInfo() { count = 0; }
				int count;
				AABB bounds;
			};
			BucketInfo buckets[BVH_BINS];

			// Initialize _BucketInfo_ for SAH partition buckets
			float denominatorInv = 1.0f / (centroidBounds.dimMax(dim) - centroidBounds.dimMin(dim));
			for (uint32_t i = start; i < end; ++i)
			{

				int b = int( BVH_BINS *
							 (buildData[i].centroid.dim(dim) - centroidBounds.dimMin(dim)) *
							 denominatorInv );
				if (b == BVH_BINS)
					b = BVH_BINS - 1;
				
				buckets[b].count++;
				buckets[b].bounds = Union(buckets[b].bounds, buildData[i].bbox);
			}

			// Compute costs for splitting after each bucket
			float cost[BVH_BINS - 1];
			for (int i = 0; i < BVH_BINS - 1; ++i)
			{
				AABB b0, b1;
				int count0 = 0, count1 = 0;
				for (int j = 0; j <= i; ++j)
				{
					b0 = Union(b0, buckets[j].bounds);
					count0 += buckets[j].count;
				}
				for (int j = i + 1; j < BVH_BINS; ++j)
				{
					b1 = Union(b1, buckets[j].bounds);
					count1 += buckets[j].count;
				}
				// The simplified version of SAH, i.e. removing constants
				cost[i] = count0*b0.area() + count1*b1.area();
			}

			// Find bucket to split at that minimizes SAH metric
			float minCost = cost[0];
			uint32_t minCostSplit = 0;
			for (int i = 1; i < BVH_BINS - 1; ++i)
			{
				if (cost[i] < minCost)
				{
					minCost = cost[i];
					minCostSplit = i;
				}
			}

			// Either create leaf or split objects at selected SAH bucket
			// The original PBRT way, stil prone to Bad Artists, see below
			if (nPrimitives > BVH_MAX_INLEAF || minCost < currentCost) 
			{
				// Divide the objects (calculate middle point) when:
				// i,  there are more objects under current node than BVH_MAX_INLEAF, or
				// ii, there is a SAH cost smaller than current cost
				// * As for the worst case, when i. is true and ii. is not, and there are  
				// * (more than BVH_MAX_INLEAF) objects to divide, errors might occur.
				BVHPrimitiveInfo *pmid = std::partition(&buildData[start],
														&buildData[end - 1] + 1,
														CompareToBucket(minCostSplit, BVH_BINS, 
																		dim, centroidBounds) );
				mid = (uint32_t)(pmid - &buildData[0]);
			}
			else
			{
				// Create a leaf node when the minimum cost is no smaller than current cost
				uint32_t firstPrimOffset = (uint32_t)orderedPrims.size();
				for (uint32_t i = start; i < end; ++i)
				{
					uint32_t primNum = buildData[i].objectIndex;
					orderedPrims.push_back(objects[primNum]);
				}
				node->initLeafNode(firstPrimOffset, nPrimitives, bbox);
				return node;
			}
		}
		// Finally made it to initInteriorNode, make this a interior and recursively build on it
		node->initInteriorNode(	dim,
								recursiveBuild(	buildData, start, mid,
												totalNodes, orderedPrims),
								recursiveBuild(	buildData, mid, end,
												totalNodes, orderedPrims));
	}
	return node;
}

// The method to flatten the normal BVH tree to a linear one
uint32_t BVH::flattenBVHTree(BVHBuildNode *node, uint32_t *offset)
{
	LinearBVHNode *linearNode = &root[*offset];
	
	linearNode->bbox = node->bbox;
	uint32_t myOffset = (*offset)++;
	if (node->nPrimitives > 0)
	{
		// Create leaf flattened BVH node
		linearNode->primitivesOffset = node->firstPrimOffset;
		linearNode->nPrimitives = node->nPrimitives;
	}
	else
	{
		// Creater interior flattened BVH node
		linearNode->axis = node->splitAxis;
		linearNode->nPrimitives = 0;
		flattenBVHTree(node->children[0], offset);
		linearNode->secondChildOffset = flattenBVHTree(node->children[1], offset);
	}
	return myOffset;
}

// The Ray<->BVH hit traversal quick check method, for BVH traversal
bool BVH::hit_check(Ray& ray)
{
	bool hit = false;

	if (!root)
		return hit;

	Vector3D invDir(1.0f / ray.d.x, 1.0f / ray.d.y, 1.0f / ray.d.z);
	uint32_t dirIsNeg[3] = { invDir.x < 0.0f, invDir.y < 0.0f, invDir.z < 0.0f };
	
	// Follow ray through BVH nodes to find primitive intersections
	uint32_t todoOffset = 0, nodeNum = 0;
	uint32_t todo[64];

	while (true)
	{
		const LinearBVHNode *node = &root[nodeNum];
		// Check ray against BVH node
		if (::rayHitBvhAABB(node->bbox, ray, invDir, dirIsNeg))
		{
			if (node->nPrimitives > 0)
			{
				hit = true;
				if (todoOffset == 0)
					break;
				nodeNum = todo[--todoOffset];
			}
			else
			{
				// Put far BVH node on _todo_ stack, advance to near node
				if (dirIsNeg[node->axis])
				{
					todo[todoOffset++] = nodeNum + 1;
					nodeNum = node->secondChildOffset;
				}
				else
				{
					todo[todoOffset++] = node->secondChildOffset;
					nodeNum = nodeNum + 1;
				}
			}
		}
		else
		{
			if (todoOffset == 0)
				break;
			nodeNum = todo[--todoOffset];
		}
	}
	return hit;
}

// The Ray<->BVH hit traversal method, for shadow rays
bool BVH::hit(Ray& ray)
{
	bool hit = false;
	
	if (!root)
		return hit;
	
	Vector3D invDir(1.0f / ray.d.x, 1.0f / ray.d.y, 1.0f / ray.d.z);
	uint32_t dirIsNeg[3] = { invDir.x < 0.0f, invDir.y < 0.0f, invDir.z < 0.0f };

	// Follow ray through BVH nodes to find primitive intersections
	uint32_t todoOffset = 0, nodeNum = 0;
	uint32_t todo[64];

	while (true)
	{
		const LinearBVHNode *node = &root[nodeNum];
		// Check ray against BVH node
		if (::rayHitBvhAABB(node->bbox, ray, invDir, dirIsNeg))
		{
			if (node->nPrimitives > 0)
			{
				// Intersect ray with primitives in leaf BVH node
				for (uint32_t i = 0; i < node->nPrimitives; ++i)
				{
					// Ckeck with all primitives
					if (objects[node->primitivesOffset + i]->hit(ray))
					{
						hit = true;
					}
				}
				if (todoOffset == 0)
					break;
				nodeNum = todo[--todoOffset];
			}
			else
			{
				// Put far BVH node on _todo_ stack, advance to near node
				if (dirIsNeg[node->axis])
				{
					todo[todoOffset++] = nodeNum + 1;
					nodeNum = node->secondChildOffset;
				}
				else
				{
					todo[todoOffset++] = node->secondChildOffset;
					nodeNum = nodeNum + 1;
				}
			}
		}
		else
		{
			if (todoOffset == 0)
				break;
			nodeNum = todo[--todoOffset];
		}
	}
	return hit;
}

// The Ray<->BVH hit traversal method, with HitPoint
bool BVH::hit(Ray& ray, HitPoint& hitPoint)
{
	bool hit = false;

	if (!root)
		return hit;

	Vector3D invDir(1.0f / ray.d.x, 1.0f / ray.d.y, 1.0f / ray.d.z);
	uint32_t dirIsNeg[3] = { invDir.x < 0.0f, invDir.y < 0.0f, invDir.z < 0.0f };

	// Follow ray through BVH nodes to find primitive intersections
	uint32_t todoOffset = 0, nodeNum = 0;
	uint32_t todo[64];

	while (true)
	{
		const LinearBVHNode *node = &root[nodeNum];
		// Check ray against BVH node
		if (::rayHitBvhAABB(node->bbox, ray, invDir, dirIsNeg))
		{
			if (node->nPrimitives > 0)
			{
				// Intersect ray with primitives in leaf BVH node
				for (uint32_t i = 0; i < node->nPrimitives; ++i)
				{
					if (objects[node->primitivesOffset + i]->hit(ray, hitPoint))
					{
						hit = true;
					}
				}
				if (todoOffset == 0)
					break;
				nodeNum = todo[--todoOffset];
			}
			else
			{
				// Put far BVH node on _todo_ stack, advance to near node
				if (dirIsNeg[node->axis])
				{
					todo[todoOffset++] = nodeNum + 1;
					nodeNum = node->secondChildOffset;
				}
				else
				{
					todo[todoOffset++] = node->secondChildOffset;
					nodeNum = nodeNum + 1;
				}
			}
		}
		else
		{
			if (todoOffset == 0)
				break;
			nodeNum = todo[--todoOffset];
		}
	}
	return hit;
}

// The RayPacket<->BVH hit traversal method
bool BVH::hit_packet(RayPacket& packet)
{
	bool hit = false;

	if (!root)
		return hit;

	// The initial firstActive Ray info
	Vector3D invDir(1.0f / packet.rays[packet.firstActive].d.x,
					1.0f / packet.rays[packet.firstActive].d.y,
					1.0f / packet.rays[packet.firstActive].d.z);
	uint32_t dirIsNeg[3] = { invDir.x < 0.0f, invDir.y < 0.0f, invDir.z < 0.0f };
	
	// Follow ray through BVH nodes to find primitive intersections
	uint32_t todoOffset = 0, nodeNum = 0;
	uint32_t todo[64];
	
	// This is for a smarter way, but not yet ready to use
	//uint32_t preActive[64];  preActive[0] = packet.firstActive;

	while (true)
	{
		const LinearBVHNode *node = &root[nodeNum];
		uint32_t newActive = 0;
		/// First hit test
		/// Check current firstActive ray against BVH Node

		if (::rayHitBvhAABB(node->bbox, packet.rays[packet.firstActive], invDir, dirIsNeg))
		{
			if (node->nPrimitives > 0)
			{
				// Intersect RayPacket.rays with primitives in leaf BVH node
				for (uint32_t p = 0; p < node->nPrimitives; ++p)
				{
					for (uint32_t r = packet.firstActive; r < RAY_PACKET_SIZE; ++r)
					{
						if (objects[node->primitivesOffset + p]->hit(packet.rays[r], packet.hitPoints[r]))
						{
							hit = true;
						}
					}
				}
				if (todoOffset == 0)
					break;
				nodeNum = todo[--todoOffset];
			}
			else
			{
				//preActive[todoOffset] = packet.firstActive;
				// Put far BVH node on _todo_ stack, advance to near node
				if (dirIsNeg[node->axis])
				{
					todo[todoOffset++] = nodeNum + 1;
					nodeNum = node->secondChildOffset;
				}
				else
				{
					todo[todoOffset++] = node->secondChildOffset;
					nodeNum = nodeNum + 1;
				}
			}
		}
		else if (packetHitBvhAABB(node->bbox, packet, newActive))
		{
			if (node->nPrimitives > 0)
			{
				// Intersect RayPacket.rays with primitives in leaf BVH node, from the new firstActive
				for (uint32_t p = 0; p < node->nPrimitives; ++p)
				{
					for (uint32_t r = newActive; r < RAY_PACKET_SIZE; ++r)
					{
						if (objects[node->primitivesOffset + p]->hit(packet.rays[r], packet.hitPoints[r]))
						{
							hit = true;
						}
					}
				}
				if (todoOffset == 0)
					break;
				nodeNum = todo[--todoOffset];
			}
			else
			{
				//preActive[todoOffset] = newActive;
				// Put far BVH node on _todo_ stack, advance to near node
				if (dirIsNeg[node->axis])
				{
					todo[todoOffset++] = nodeNum + 1;
					nodeNum = node->secondChildOffset;
				}
				else
				{
					todo[todoOffset++] = node->secondChildOffset;
					nodeNum = nodeNum + 1;
				}
			}
		}
		else
		{	
			if (todoOffset == 0)
				break;
			nodeNum = todo[--todoOffset];
		}
	}
	return hit;
}

// Get how many nodes in current BVH
int BVH::node_count() const
{
	return this->number;
}