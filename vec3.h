#ifndef VEC3_H_DEFINED
#define VEC3_H_DEFINED


#include <math.h>

union vec3
{
	vec3() = default;
	vec3(float e0, float e1, float e2) : e{ e0,e1,e2 } {}
	inline float operator[](int i) const { return e[i]; }
	inline float &operator[](int i) { return e[i]; }
	inline vec3 operator-() const { return vec3(-x, -y, -z); }
	vec3 &operator+=(const vec3 &other);
	inline vec3 &operator-=(const vec3 &other);
	inline vec3 &operator/=(const vec3 &other);
	inline vec3 &operator*=(const vec3 &other);
	inline vec3 &operator*=(float other);
	inline vec3 &operator/=(float other);
	inline vec3 operator+(const vec3 &other) const;
	vec3 operator/(const vec3 &other) const;
	vec3 operator*(const vec3 &other) const;
	inline float length() const;
	inline float squaredLength() const;
	void normalize();
	inline vec3 normalized() const;
	inline vec3 rotateX(float angle);
	inline vec3 rotateY(float angle);
	inline vec3 rotateZ(float angle);

	static float dot(const vec3 &v1, const vec3 &v2);
	static vec3 cross(const vec3 &v1, const vec3 &v2);
	static vec3 lerp(const vec3 &v1, const vec3 &v2, float t);
	static vec3 reflect(const vec3 &incident, const vec3 &normal);
	static bool refract(const vec3 &incident, const vec3 &normal, float niOverNt, vec3 &refracted);
	

	struct
	{
		float x, y, z;
	};
	struct
	{
		float r, g, b;
	};
	float e[3]{};
};

inline vec3 operator*(vec3 v, float f)
{
	return vec3(v.x * f, v.y * f, v.z * f);
}

inline vec3 operator-(const vec3 &v1, const vec3 &v2)
{
	return vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

inline vec3 operator/(const vec3 &v1, float f)
{
	return vec3(
		v1.x / f,
		v1.y / f,
		v1.z / f);
}

inline vec3 vec3::rotateX(float angle)
{
	return vec3(x, y * cosf(angle) - z * sinf(angle), y * sinf(angle) + z * cosf(angle));
}

inline vec3 vec3::rotateY(float angle)
{
	return vec3(x * cosf(angle) + z * sinf(angle), y, -x * sinf(angle) + z * cosf(angle));
}

inline vec3 vec3::rotateZ(float angle)
{
	return vec3(x * cosf(angle) - y * sinf(angle), x * sinf(angle) + y * cosf(angle), z);
}

#endif