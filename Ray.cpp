#include "precomp.h"
#include "Ray.h"

// Assignment Operator
Ray& Ray::operator= (const Ray& rayInput)
{
	if (this == &rayInput)
		return *this;
	o = rayInput.o;
	d = rayInput.d;
	counter = rayInput.counter;
	return *this;
}

