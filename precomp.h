#define SCRWIDTH			256*2
#define SCRHEIGHT			256*2
// #define FULLSCREEN
#define GLM_FORCE_RADIANS
// #define OLDTEMPLATESTYLE
//#define SHOWI_NFO			// Show infomation such as time per frame, FPS, scene info at the rendering port
#define IPI				// Enable image processing
#define LOAD_OBJ

#define PI					3.1415926535897932384626433832795028841972f
#define TWO_PI				6.2831853071795864769232837665590057683943f
#define invPI				0.3183098861837906715f
#define invTWO_PI			0.1591549430918953358f
#define PI_180				0.0174532925199432957f
#define EPSILON				0.0001f
#define VERYBIG				1000000000.0f
#define AMBIENT				0.25f
#define RAYDEPTH			8
#define GLASS_N				1.200f
#define WATER_N				1.333f
#define GLASS_CRITICAL		0.9851107833377457f
#define WATER_CRITICAL		0.8483456688217663f
#define GLASS_R0			0.008264462809917356f
#define WATER_R0			0.020373187841971424f
#define AIR_GLASS			0.8333333333333334f
#define AIR_GLASS2			0.6944444444444445f
#define GLASS_AIR			1.200f
#define GLASS_AIR2			1.440f
#define BVH_BINS			8
#define BVH_MAX_INLEAF		4
#define RAY_PACKET_SIZE		64							// 8*8
#define RAY_PACKET_SQRT		8
#define SAMPLEPERPIXEL		16							// Notice in each pixel SPP*SPP samples are generated
#define	SPP					256							// 16*16
#define drand48()			(float)rand()/RAND_MAX
#define NEE												// Next event estimation
//#define VIS_CACHE										// The Visibility Caching switch
#define BIG_LIGHT
//#define BIGGER_LIGHT
#define INDOOR
#define FIRST_BOUNCE									// Do not use the vis cache at first bounce
#define USE_CACHE_SINCE		0							// Start using the cache since which depth of rays
#define	NUM_VIS_CACHE		4096						// 2k -> ~32mb		4k	-> ~64 mb	10k		-> ~160 mb
#define	NUM_CACHE_CAND		1024000*2					// 200k -
#define CACHE_CAND_LIGHT	1024000						// 100k, half
#define	CACHE_CAND_EYE		1024000						// 100k, half
#define	VIS_CACHE_SM_SIZE	64							// Shadow map resolution 64x64
#define	PI_OVER_SM_SIZE_1	0.09973310011396169f		// 64x64: 2pi/(64-1)	
#define PI_OVER_SM_SIZE_2	0.02493327502849042f		// 64x64: (pi/2)/(64-1)
#define SM_SIZE_OVER_PI_1	10.026761414789407f			// 64x64: (64-1)/2pi
#define SM_SIZE_OVER_PI_2	40.10704565915763f			// 64x64: (64-1)/(pi/2)

#if 0
#define VIS_GRID_SIZE		64							// Maximum grid size: 64x64x64
#elif 1
#define VIS_GRID_SIZE		128							// Maximum grid size: 128x128x128
#else
#define VIS_GRID_SIZE		256							// Maximum grid size: 256x256x256
#endif
#define VIS_MAX_PT_VOX		6							// Maximum cache points in one voxel, since each voxel has six faces

#include <inttypes.h>
extern "C" 
{ 
#include "glew.h" 
}
#include "gl.h"
#include "io.h"
#include <ios>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <time.h>
#include "SDL.h"
#include "SDL_syswm.h"
#include "wglext.h"
#include "template.h"
#include "counters.h"
#include "surface.h"

// All utilities stuff
#include "Matrix.h"
#include "Normal.h"
#include "Point2D.h"
#include "Point3D.h"
#include "Vector3D.h"
#include "Ray.h"
#include "RGBColor.h"
#include "HitPoint.h"
#include "AABB.h"
#include "Camera.h"
#include "BVH.h"
#include "VisCache.h"
#include "Scene.h"
#include "Tracer.h"
#include "Sampler.h"
#include "ImageProcessing.h"

// All shading stuff
#include "Material.h"
#include "Texture.h"
#include "Light.h"
#include "EnvBall.h"

// All primitives stuff
#include "GeoPrimitive.h"
#include "Plane.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Rect.h"

using namespace std;
using namespace Tmpl8;				// to use template classes
//using namespace glm;				// to use glm vector stuff

#include <vector>
#include "game.h"
#include "fcntl.h"
#include "threads.h"
//#include <glm/gtc/matrix_transform.hpp>
#include "freeimage.h"