#ifndef HITABLE_H_DEFINED
#define HITABLE_H_DEFINED
#include "ray.h"
#include "AABB.h"

class Material;

struct hitRecord
{
	float distance = {};
	vec3 point = {};
	vec3 exitPoint = {};
	vec3 normal = {};
	Material *material = nullptr;
	float u = {}, v = {};
};

class Hitable
{
public:
	~Hitable(){}
	virtual bool hit(const ray& givenRay, float minT, float maxT, hitRecord& record) const = 0;
	virtual bool boundingBox(AABB3 &aabb) const = 0;
};

#endif // !HITABLE_H_DEFINED





