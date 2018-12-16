#ifndef VEC3_HPP
#define VEC3_HPP

#include <ostream>
#include <istream>

class Vec3 {
public:
	Vec3();
	Vec3(float x, float y, float z);
	Vec3(const Vec3& other);
	Vec3& operator+=(const Vec3& other);
	Vec3& operator-=(const Vec3& other);
	Vec3& operator*=(const Vec3& other);
	Vec3& operator*=(float scalar);
	Vec3& operator/=(float scalar);
	Vec3& normalize();
	friend Vec3 operator+(const Vec3& v1, const Vec3& v2);
	friend Vec3 operator-(const Vec3& v1, const Vec3& v2);
	friend Vec3 operator*(const Vec3& v1, const Vec3& v2);
	friend Vec3 operator*(float scalar, const Vec3& v2);
	friend Vec3 operator*(const Vec3& v2, float scalar);
	friend Vec3 operator/(const Vec3& v1, float scalar);
	friend Vec3 operator-(const Vec3& v);
	friend bool operator==(const Vec3& v1, const Vec3& v2);
	friend bool operator!=(const Vec3& v1, const Vec3& v2);
	float& operator[](int index);
	float operator[](int index) const;
	float length() const;
	static float dotProduct(const Vec3& v1, const Vec3& v2);
	static Vec3 crossProduct(const Vec3& v1, const Vec3& v2);
	static Vec3 normalize(Vec3 v);
	static Vec3 project(const Vec3& v, const Vec3& onto);
	static Vec3 refract(const Vec3& v, const Vec3& normal, float index1, float index2);
	static float angleBetween(const Vec3& v1, const Vec3& v2);
	static void orthonormalize(Vec3& x, Vec3& y, Vec3& z);
	friend std::ostream& operator<<(std::ostream& out, const Vec3& v);
	friend std::istream& operator>>(std::istream& in, Vec3& v);

	float x, y, z;
};

#endif