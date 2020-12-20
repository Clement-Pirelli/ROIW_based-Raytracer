#ifndef DIELECTRIC_MATERIAL_H_DEFINED
#define DIELECTRIC_MATERIAL_H_DEFINED
#include "Material.h"
#include "randUtils.h"

inline float schlickApproximation(float angleCosine, float refractionIndex)
{
	float r0 = (1.0f - refractionIndex) / (1.0f + refractionIndex);
	r0 *= r0;
	return r0 + (1.0f - r0) * pow(1.0f - angleCosine, 5.0f);
}


class Dielectric : public Material
{
public:
	Dielectric(float givenRefractionIndex) : refractionIndex(givenRefractionIndex) {}

	virtual bool scatter(const ray &inRay, const hitRecord &record, vec3 &attenuation, ray &scatteredRay, float firstRandom, float secondRandom) const override
	{
		vec3 outwardNormal = {};
		vec3 reflected = vec3::reflect(inRay.direction, record.normal);
		float niOverNt = .0f;
		attenuation = vec3(1.0f, 1.0f, 1.0f);
		vec3 refracted = {};
		float reflectProbability = .0f;
		float cosine = .0f;
		if(vec3::dot(inRay.direction, record.normal) > .0f)
		{
			outwardNormal = -record.normal;
			niOverNt = refractionIndex;
			cosine = refractionIndex * vec3::dot(inRay.direction, record.normal);
		} else 
		{
			outwardNormal = record.normal;
			niOverNt = 1.0f / refractionIndex;
			cosine = -vec3::dot(inRay.direction, record.normal);
		}

		if(vec3::refract(inRay.direction, outwardNormal, niOverNt, refracted))
		{
			reflectProbability = schlickApproximation(cosine, refractionIndex);
		} else 
		{
			reflectProbability = 1.0f;
		}

		if(drand48() < reflectProbability)
		{
			scatteredRay = ray(record.point, reflected);
		} else 
		{
			scatteredRay = ray(record.point, refracted);
		}

		return true;
	}

	float refractionIndex;
};

#endif // !DIELECTRIC_MATERIAL_H_DEFINED
