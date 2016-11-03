#pragma once

#include "RGBColor.h"		// Forward declaration does not work!
#include "HitPoint.h"

class Point3D;
class Vector3D;

class Ray
{
public:
	union{ struct{ Point3D o; Vector3D d; float t; uint32_t counter; }; float cell[8]; };
	//// Origin of the Ray
	//Point3D o;
	//// Direction of the Ray
	//Vector3D d;
	//// Ray parameter
	//float t;
	//// Current Depth of the Ray
	////int depth;
	//// Test Counter, also used as a depth value of current ray when not used as a counter of hit
	//uint32_t counter;

	// Constructors
	// Defult Constructor
	// Constructs a Ray at Orgin pointing in Z direction
	inline Ray(){ o.x = 0.0f; o.y = 0.0f; o.z = 0.0f; d.x = 0.0f; d.y = 0.0f; d.z = 1.0f; t = VERYBIG; counter = 0; }
	// Construct from Origin and Direction
	inline Ray(const Point3D origin, const Vector3D direction)
		: o(origin), d(direction), t(VERYBIG), counter(0)
	{
		o = origin; d = direction; t = VERYBIG; counter = 0;
		d.normalize();
	}
	// Copy Constructor
	inline Ray(const Ray& rayInput)
	{
		o = rayInput.o; d = rayInput.d; t = rayInput.t; 
		counter = rayInput.counter;
	}
	// Destructor
	~Ray(){}

	// Assignment Operator
	Ray& operator= (const Ray& rayInput);
	// Return current point / possition
	inline Point3D curPos(){ return o + t*d; }

};

struct RayPacket
{
	// A bunch of Ray's
	Ray rays[RAY_PACKET_SIZE];
	// The first ray item index
	// the last ray item = firstItem + RAY_PACKET_SIZE * RAY_PACKET_SIZE;
	uint32_t firstActive;			// , previousActive;
	// The frustum of current packet, implemented using 4 Ray indices //Ray frustum[4];
	uint32_t frustum[4];			// <<----- Consider revising, index 0 and index 3 are known
	// The RGBColor array corresponding to rays of same indices
	RGBColor colors[RAY_PACKET_SIZE];
	// The HitPoints corresponding to Rays, notice this is only for primary ray
	HitPoint hitPoints[RAY_PACKET_SIZE];
};