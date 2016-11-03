#include "precomp.h"
#include "GeoPrimitive.h"


GeoPrimitive::GeoPrimitive()
{
}


GeoPrimitive::~GeoPrimitive()
{
}

void GeoPrimitive::set_material(Material* mat)
{
	this->material = mat;
	// If this material is a lighting Obj, add primitive ptr to Material
	if (mat->isLight)
	{
		mat->link_to_mat(this);
	}	
}