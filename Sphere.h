#ifndef SPHERE_H_DEFINED
#define SPHERE_H_DEFINED
#include "Hitable.h"
#include "Material.h"

class Sphere : public Hitable
{
public:
	Sphere() = default;
	Sphere(const vec3& givenCenter, float givenRadius, Material *givenMaterial) : center(givenCenter), radius(givenRadius), material(givenMaterial) {}
	
	
	virtual bool hit(const ray& givenRay, float minT, float maxT, hitRecord& record) const override;
	virtual bool boundingBox(AABB &aabb) const override;
	
	vec3 center;
	float radius;
	Material *material = nullptr;
};

#endif
