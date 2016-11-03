#include "precomp.h"
#include "VisCache.h"

void VisCache::XYZ2UVW(const Point3D& xyz, int& u, int& v, int& w)
{
	float uf = (xyz.x - worldBBox.x0) / m_grid.delta;
	float vf = (xyz.y - worldBBox.y0) / m_grid.delta;
	float wf = (xyz.z - worldBBox.z0) / m_grid.delta;

	u = (int)uf < m_grid.grid_dim[0] - 1 ? (int)uf : m_grid.grid_dim[0] - 1;
	v = (int)vf < m_grid.grid_dim[1] - 1 ? (int)vf : m_grid.grid_dim[1] - 1;
	w = (int)wf < m_grid.grid_dim[2] - 1 ? (int)wf : m_grid.grid_dim[2] - 1;
}

// Query the grid with offset, neighbours, ray and normal
bool VisCache::query(int queryPointOffset, int* queryNeighbours, Ray& ray, Normal& nrml, float& result)
{
#if 0		// Use neighbours
	float R0, R1, R2, R3, R4, R5, R6;
	bool  B0, B1, B2, B3, B4, B5, B6;
	
	B0 = query(queryPointOffset, ray, nrml, R0);
	B1 = query(*queryNeighbours, ray, nrml, R1);		queryNeighbours++;
	B2 = query(*queryNeighbours, ray, nrml, R2);		queryNeighbours++;
	B3 = query(*queryNeighbours, ray, nrml, R3);		queryNeighbours++;
	B4 = query(*queryNeighbours, ray, nrml, R4);		queryNeighbours++;
	B5 = query(*queryNeighbours, ray, nrml, R5);		queryNeighbours++;
	B6 = query(*queryNeighbours, ray, nrml, R6);		queryNeighbours++;
	
	result = (0.4f*R0*B0 + 0.1f*R1*B1 + 0.1f*R2*B2 + 0.1f*R3*B3 + 0.1f*R4*B4 + 0.1f*R5*B5 + 0.1f*R6*B6) / 
			 (0.4f*B0 + 0.1f*B1 + 0.1f*B2 + 0.1f*B3 + 0.1f*B4 + 0.1f*B5 + 0.1f*B6);
	// No useful cache or no cache
	if (B0 == false && B1 == false && B2 == false && B3 == false && B4 == false && B5 == false && B6 == false)
	{
		// Set to -1 to invoke regular BVH traversal
		result = -1.0f;
		return false;
	}

	// Occuluded according to cache
	if (!MoreOrLess(ray.t, result))
		return false;
	
	return true;
#else		// Do not use neighbours
	float R0;	bool  B0;
	B0 = query(queryPointOffset, ray, nrml, R0);
	result = R0;
	// No useful cache or no cache
	if (B0 == false)
	{
		// Set to -1 to invoke regular BVH traversal
		result = -1.0f;
		return false;
	}

	// Occuluded according to cache
	if (!MoreOrLess(ray.t, result))
		return false;

	return true;
#endif
}
bool VisCache::query(int offset, Ray& ray, Normal& nrml, float& result)
{
	//if (offset >= m_grid.count)	return false;
	if (m_grid.data[offset].cache_in_voxel == 0) return false;
	
	float tmp = 0.0f; 	float accum = 0.0f; 	int counter = 0;
	// Loop through all available cache's
	for (int i = 0; i < m_grid.data[offset].cache.size(); i++)
	{
		float dotProd = m_grid.data[offset].cache[i]->n * nrml;
		// Check if current cache is similar in orientation
		if (dotProd <= 0.707106f)
			continue;

		int u = 0, v = 0;
		Vector3D dirInWorldSpace = ray.curPos() - m_grid.data[offset].cache[i]->p;	dirInWorldSpace.normalize();
		ONB ONB_helper; ONB_helper.build_onb(m_grid.data[offset].cache[i]->n);
		Vector3D dirInCacheSpace = ONB_helper.de_local(dirInWorldSpace);			dirInCacheSpace.normalize();

		XYZ2UV(dirInCacheSpace.x, dirInCacheSpace.y, dirInCacheSpace.z, u, v);
		if (u < 0 || u > VIS_CACHE_SM_SIZE || v < 0 || v > VIS_CACHE_SM_SIZE)	continue;

		// Use dot product as weight, which make sense because closer in orientation, more weight should be given
		counter++;
		accum += dotProd;
		tmp += dotProd * m_grid.data[offset].cache[i]->sm.value[v * VIS_CACHE_SM_SIZE + u];
	}
	if (counter == 0)	return false; 	if (accum == 0.0f)	return false;
	// Normalize the result
#if 1
	result = tmp / counter;
#else
	result = tmp / accum;
#endif
	return true;
}

/// Functions to map spatial dir into shadowmap coordinates
void XYZ2UV(const float &x, const float &y, const float &z, int &u, int &v)
{
	float phi = acosf(y);
	float theta = z >= 0.0f ? acosf(x / sqrtf(1 - y*y)) : TWO_PI - acosf(x / sqrtf(1 - y*y));

	float temp_u = theta * SM_SIZE_OVER_PI_1;
	float temp_v = phi * SM_SIZE_OVER_PI_2;

	u = (int)temp_u; v = (int)temp_v;
}
void UV2XYZ(const int &u, const int &v, float &x, float &y, float &z)
{
	float theta = PI_OVER_SM_SIZE_1 * u;
	float phi = PI_OVER_SM_SIZE_2 * v;
	x = sinf(phi) * cosf(theta);
	y = cosf(phi);
	z = sinf(phi) * sinf(theta);
}