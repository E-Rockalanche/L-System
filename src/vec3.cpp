/*
vec3.cpp
Eric Roberts
*/

#include "vec3.hpp"
#include <stdexcept>
#include <cmath>
#include <ostream>
#include <istream>

Vec3::Vec3() {
	x = 0.0;
	y = 0.0;
	z = 0.0;
}

Vec3::Vec3(float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

Vec3::Vec3(const Vec3& other) {
	x = other.x;
	y = other.y;
	z = other.z;
}

Vec3& Vec3::operator+=(const Vec3& other){
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

Vec3& Vec3::operator-=(const Vec3& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

Vec3& Vec3::operator*=(const Vec3& other) {
	x *= other.x;
	y *= other.y;
	z *= other.z;
	return *this;
}

Vec3& Vec3::operator*=(float scalar) {
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

Vec3& Vec3::operator/=(float scalar) {
	x /= scalar;
	y /= scalar;
	z /= scalar;
	return *this;
}

Vec3& Vec3::normalize() {
	float my_length = length();
	if (my_length > 0) {
		x /= my_length;
		y /= my_length;
		z /= my_length;
	}
	return *this;
}

Vec3 operator+(const Vec3& v1, const Vec3& v2) {
	Vec3 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}

Vec3 operator-(const Vec3& v1, const Vec3& v2) {
	Vec3 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

Vec3 operator*(const Vec3& v1, const Vec3& v2) {
	Vec3 result;
	result.x = v1.x * v2.x;
	result.y = v1.y * v2.y;
	result.z = v1.z * v2.z;
	return result;
}

Vec3 operator*(float scalar, const Vec3& v) {
	Vec3 result;
	result.x = v.x * scalar;
	result.y = v.y * scalar;
	result.z = v.z * scalar;
	return result;
}

Vec3 operator*(const Vec3& v, float scalar) {
	Vec3 result;
	result.x = v.x * scalar;
	result.y = v.y * scalar;
	result.z = v.z * scalar;
	return result;
}

Vec3 operator/(const Vec3& v, float scalar) {
	Vec3 result;
	result.x = v.x / scalar;
	result.y = v.y / scalar;
	result.z = v.z / scalar;
	return result;
}

Vec3 operator-(const Vec3& v) {
	Vec3 result;
	result.x = -v.x;
	result.y = -v.y;
	result.z = -v.z;
	return result;
}

bool operator==(const Vec3& v1, const Vec3& v2) {
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

bool operator!=(const Vec3& v1, const Vec3& v2) {
	return !(v1 == v2);
}

float& Vec3::operator[](int index) {
	switch(index) {
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			throw std::runtime_error("Vec3 index out of bounds: "+std::to_string(index));
	}
}

float Vec3::operator[](int index) const {
	switch(index) {
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			throw std::runtime_error("Vec3 index out of bounds: "+std::to_string(index));
	}
}

float Vec3::length() const {
	return std::sqrt(x*x + y*y + z*z);
}

float Vec3::dotProduct(const Vec3& v1, const Vec3& v2) {
	return v1.x * v2.x
		+ v1.y * v2.y
		+ v1.z * v2.z;
}

Vec3 Vec3::crossProduct(const Vec3& v1, const Vec3& v2) {
	Vec3 result;
	for(int i = 0; i < 3; i++) {
		int j = (i+1)%3;
		int k = (i+2)%3;
		result[i] = v1[j] * v2[k] - v1[k] * v2[j];
	}
	return result;
}

Vec3 Vec3::normalize(Vec3 v) {
	return v.normalize();
}

Vec3 Vec3::project(const Vec3& v, const Vec3& onto) {
	return Vec3::dotProduct(v, onto) / Vec3::dotProduct(onto, onto) * onto;
}

Vec3 Vec3::refract(const Vec3& v, const Vec3& normal, float index1, float index2) {
	Vec3 proj = Vec3::project(v, normal);
	Vec3 ortho = v - proj;
	return Vec3::normalize(proj + (index1 / index2) * ortho);
}

float Vec3::angleBetween(const Vec3& v1, const Vec3& v2) {
	return std::acos(Vec3::dotProduct(v1, v2) / (v1.length() * v2.length()));
}

void Vec3::orthonormalize(Vec3& x, Vec3& y, Vec3& z) {
	x.normalize();
	y -= Vec3::project(y, x);
	y.normalize();
	z -= Vec3::project(z, x) + Vec3::project(z, y);
	z.normalize();
}

std::ostream& operator<<(std::ostream& out, const Vec3& v) {
	out << v.x << ' ' << v.y << ' ' << v.z;
	return out;
}

std::istream& operator>>(std::istream& in, Vec3& v) {
	in >> v.x >> v.y >> v.z;
	return in;
}