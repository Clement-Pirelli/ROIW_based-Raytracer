#ifndef PBR_MATERIAL_H_DEFINED
#define PBR_MATERIAL_H_DEFINED
#include "Material.h"
#include "randUtils.h"
#include "Texture.h"

class PBRMaterial : public Material
{
public:
	PBRMaterial(Texture* givenAlbedo, Texture* givenEmissive, Texture* givenNormals, Texture* givenRoughness) 
	: albedo(givenAlbedo)
	, emissive(givenEmissive)
	, normals(givenNormals)
	, roughness(givenRoughness){}

	virtual bool scatter(const ray &inRay, const hitRecord &record, vec3 &attenuation, ray &scatteredRay, float firstRandom, float secondRandom) const override
	{
		//const vec3 mapNormal = (normals->valueAt(record.u, record.v, record.point)*2.0f - 1.0f).normalized();
		const vec3 roughnessAtUV = roughness->valueAt(record.u, record.v, record.point);
		attenuation = albedo->valueAt(record.u, record.v, record.point);

		if(drand48() > roughnessAtUV.r())
		{
			const vec3 target = mapRectToCosineHemisphere(record.normal, firstRandom, secondRandom);
			scatteredRay = ray(record.point, target);
			return true;
		} else
		{
			const vec3 reflected = vec3::reflect(inRay.direction, record.normal);
			const vec3 newRayDirection = (reflected + randInUnitSphere() * roughnessAtUV).normalized();
			scatteredRay = ray(record.point, newRayDirection);

			return vec3::dot(scatteredRay.direction, record.normal) > .0f;
		}
	}

	virtual vec3 emitted(float u, float v, vec3 &point) const override
	{
		return emissive->valueAt(u, v, point); 
	}

	Texture *albedo;
	Texture *emissive;
	Texture *normals; 
	Texture *roughness;
};

#endif // !METAL_MATERIAL_H_DEFINED
