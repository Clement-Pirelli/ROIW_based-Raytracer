#ifndef DIFFUSE_LIGHT_MATERIAL_H_DEFINED
#define DIFFUSE_LIGHT_MATERIAL_H_DEFINED
#include "Material.h"
#include "randUtils.h"
#include "Texture.h"

class DiffuseLight : public Material
{
public:
	DiffuseLight(Texture *givenEmissionTexture) : emissionTexture(givenEmissionTexture) {}

	virtual bool scatter(const ray &inRay, const hitRecord &record, vec3 &attenuation, ray &scatteredRay) const override
	{
		return false;
	}

	virtual vec3 emitted(float u, float v, vec3 &point) const override
	{
		return emissionTexture->valueAt(u,v,point);
	}

private:
	Texture *emissionTexture;
};

#endif