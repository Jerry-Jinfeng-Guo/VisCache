#include "precomp.h"
#include "Point2D.h"

// Defult Constructor
Point2D::Point2D()
			: x(0.0f), y(0.0f)
{
}

// Constructor with one vlaue
Point2D::Point2D(const float value)
			: x(value), y(value)
{
}

// Constructor with x and y values
Point2D::Point2D(const float x_value, const float y_value)
			: x(x_value), y(y_value)
{
}
// Copy Constructor
Point2D::Point2D(Point2D &p2d)
			: x(p2d.x), y(p2d.y)
{
}

// Assign operator
Point2D& Point2D::operator=(const Point2D&rhs)

{
	x = rhs.x;
	y = rhs.y;
	return *this;
}

// Multiplication by a float on the right
// without changing x and y values
Point2D Point2D::operator*(const float a)
{
	return (Point2D(x*a, y*a));
}

// Get coornidate value according to dimension index, 0:=X, 1:=Y
float Point2D::dim(int d)
{
	switch (d)
	{
	case 0:
		return this->x;
	case 1:
	default:
		return this->y;
	}
}


// Destructor
Point2D::~Point2D()
{
}