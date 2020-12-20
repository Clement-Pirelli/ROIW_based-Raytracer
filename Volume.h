#ifndef VOLUME_MATERIAL_H_DEFINED
#define VOLUME_MATERIAL_H_DEFINED
#include "Material.h"
#include "randUtils.h"
#include "Texture.h"

constexpr float E = 2.71828f;

class Volume : public Material
{
public:
	Volume(Texture *givenAlbedo) : albedo(givenAlbedo) {}

	virtual bool scatter(const ray &inRay, const hitRecord &record, vec3 &attenuation, ray &scatteredRay, float firstRandom, float secondRandom) const override
	{
		const vec3 target = randOnUnitSphere(firstRandom, secondRandom);
		scatteredRay = ray(record.point, target);
		attenuation = albedo->valueAt(record.u, record.v, record.point);
		return true;
	}

	Texture *albedo;
};

#endif // !METAL_MATERIAL_H_DEFINED
