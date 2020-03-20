#include "vec3.h"

vec3 &vec3::operator+=(const vec3 &other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

inline vec3 &vec3::operator-=(const vec3 &other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

inline vec3 &vec3::operator/=(const vec3 &other)
{
	x /= other.x;
	y /= other.y;
	z /= other.z;
	return *this;
}

inline vec3 &vec3::operator*=(const vec3 &other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
	return *this;
}

inline vec3 &vec3::operator*=(float other)
{
	x *= other;
	y *= other;
	z *= other;
	return *this;
}

inline vec3 &vec3::operator/=(float other)
{
	x /= other;
	y /= other;
	z /= other;
	return *this;
}

inline vec3 vec3::operator+(const vec3 &other) const
{
	return vec3(
		x + other.x,
		y + other.y,
		z + other.z);
}

vec3 vec3::operator/(const vec3 &other) const
{
	return vec3(
		x / other.x,
		y / other.y,
		z / other.z);
}

vec3 vec3::operator*(const vec3 &other) const
{
	return vec3(
		x * other.x,
		y * other.y,
		z * other.z);
}

inline float vec3::length() const
{
	return sqrtf((*this).squaredLength());
}

inline float vec3::squaredLength() const
{
	return dot(*this, *this);
}

void vec3::normalize()
{
	(*this) /= (*this).length();
}

inline vec3 vec3::normalized() const
{
	return (*this) / (*this).length();
}

float vec3::dot(const vec3 &v1, const vec3 &v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vec3 vec3::cross(const vec3 &v1, const vec3 &v2)
{
	return vec3(
		v1.e[1] * v2.e[2] - v1.e[2] * v2.e[1],
		-(v1.e[0] * v2.e[2] - v1.e[2] * v2.e[0]),
		v1.e[0] * v2.e[1] - v1.e[1] * v2.e[0]
	);
}

vec3 vec3::lerp(const vec3 &v1, const vec3 &v2, float t)
{
	return v1 * (1.0f - t) + v2 * t;
}

vec3 vec3::reflect(const vec3 &incident, const vec3 &normal)
{
	return incident - (normal * dot(incident, normal) * 2.0f);
}

bool vec3::refract(const vec3 &incident, const vec3 &normal, float niOverNt, vec3 &refracted)
{
	vec3 uv = incident.normalized();
	float dt = vec3::dot(uv, normal);
	float discriminant = 1.0f - niOverNt * niOverNt * (1.0f - dt * dt);
	if(discriminant > .0f)
	{
		refracted = (uv - normal * dt) * niOverNt - normal * sqrtf(discriminant);
		return true;
	}
	return false;
}
