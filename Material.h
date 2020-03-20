#ifndef MATERIAL_H_DEFINED
#define MATERIAL_H_DEFINED
#include "ray.h"
#include "Hitable.h"

class Material
{
public:
	virtual bool scatter(const ray &inRay, const hitRecord &rec, vec3 &attenuation, ray &scatteredRay, float firstRandom, float secondRandom) const = 0;
	virtual vec3 emitted(float u, float v, vec3 &point) const { return vec3(); }
};

#endif // !MATERIAL_H_DEFINED

