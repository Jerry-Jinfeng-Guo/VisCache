#pragma once

// Point2D is used for calculations like pixel samples
// Where things are 'floats' <-- in fact, everything runs at 'float'

class Point2D
{
public:
	union { struct { float x, y; }; float cell[2]; };
public:
	// Constructors
	// Defult Constructor
	Point2D();
	// Constructor with one vlaue
	Point2D(const float value);
	// Constructor with x and y values
	Point2D(const float x_value, const float y_value);
	// Copy Constructor
	Point2D(Point2D &p2d);

	// Assign operator
	Point2D& operator=(const Point2D&rhs);
	// Multiplication by a float on the right
	Point2D operator*(const float a);
	// Get coornidate value according to dimension index, 0:=X, 1:=Y
	float dim(int d);
	~Point2D();
};

