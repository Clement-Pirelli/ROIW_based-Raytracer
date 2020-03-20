#ifndef LAMBERTIAN_MATERIAL_H_DEFINED
#define LAMBERTIAN_MATERIAL_H_DEFINED
#include "Material.h"
#include "randUtils.h"
#include "Randomizer.h"
#include "Texture.h"
#include <cmath>

class Lambertian : public Material
{
public:
	Lambertian(Texture *givenAlbedo) : albedo(givenAlbedo) {}

	virtual bool scatter(const ray &inRay, const hitRecord &record, vec3 &attenuation, ray &scatteredRay, float firstRandom, float secondRandom) const override 
	{
		vec3 target = mapRectToCosineHemisphere(record.normal, firstRandom, secondRandom);
		scatteredRay = ray(record.point, target);
		attenuation = albedo->valueAt(record.u, record.v, record.point);
		return true;
	}

	Texture *albedo;
};

#endif // !LAMBERTIAN_MATERIAL_H_DEFINED
