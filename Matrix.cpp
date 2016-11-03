#include <math.h>
#include "precomp.h"
#include "Matrix.h"

// Defult Constructor
Matrix::Matrix()
{
	// Construct a defult matrix, which is a identity matrix
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			if (x == y)
			{
				m[x][y] = 1.0;
			}
			else
			{
				m[x][y] = 0.0;
			}
		}
	}
}
// Copy Constructor
Matrix::Matrix(const Matrix& mat)
{
	// Construct a defult matrix, which copies from the input matrix
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			m[x][y] = mat.m[x][y];
		}
	}
}
// Assignment Operator
Matrix& Matrix::operator=(const Matrix& mat)
{
	if (this == &mat)
		return *this;
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			m[x][y] = mat.m[x][y];
		}
	}
	return *this;
}
// Multiplication of two matrices
Matrix Matrix::operator*(const Matrix& mat) const
{
	Matrix out;
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{		
			float temp = 0.0;
			for (int i = 0; i < 4; i++)
			{
				temp += m[x][i] * mat.m[i][y];
			}
			out.m[x][y] = temp;
		}
	}
	return out;
}
// Multiplication by a float
Matrix Matrix::operator*(const float value)
{
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			m[x][y] *= value;
		}
	}
	return *this;
}
// Division by a float
Matrix Matrix::operator/(const float value)
{
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			m[x][y] /= value;
		}
	}
	return *this;
}
// Set to identity
void Matrix::set_identity()
{
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			if (x == y)
			{
				m[x][y] = 1.0;
			}
			else
			{
				m[x][y] = 0.0;
			}
		}
	}
}

// Destructor
Matrix::~Matrix()
{
}