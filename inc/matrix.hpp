/*
matrix.hpp
Eric Roberts
*/

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "vec3.hpp"

class Matrix {
public:
	float data[16];
	Matrix();
	float& operator[](int index) { return data[index]; }
	float operator[](int index) const { return data[index]; }
	Matrix operator*(const Matrix& other) const;
	Vec3 operator*(const Vec3& v) const;
	friend Vec3 operator*(const Vec3& v, const Matrix& m);
};

Matrix calcRotationMatrix(float angle, const Vec3& axis);

#endif
