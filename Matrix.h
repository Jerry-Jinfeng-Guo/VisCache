#pragma once

class Matrix
{
public:
	// The four by four float array
	union{ float m[4][4]; float m_flat[16]; };

	// Constructors
	// Defult Constructor
	Matrix();
	// Copy Constructor
	Matrix(const Matrix& mat);
	// Assignment Operator
	Matrix& operator=(const Matrix& mat);
	// Multiplication of two matrices
	Matrix operator*(const Matrix& mat) const;
	// Multiplication by a float
	Matrix operator*(const float value);
	// Division by a float
	Matrix operator/(const float value);
	// Set to identity
	void set_identity();

	~Matrix();
};