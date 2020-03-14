#ifndef LAMBERTIAN_MATERIAL_H_DEFINED
#define LAMBERTIAN_MATERIAL_H_DEFINED
#include "Material.h"
#include "randUtils.h"
#include "Texture.h"

class Lambertian : public Material
{
public:
	Lambertian(Texture *givenAlbedo) : albedo(givenAlbedo) {}

	virtual bool scatter(const ray &inRay, const hitRecord &record, vec3 &attenuation, ray &scatteredRay) const override 
	{
		vec3 target = record.normal + randInUnitSphere();
		scatteredRay = ray(record.point, target);
		attenuation = albedo->valueAt(0.0f,.0f,record.point);
		return true;
	}


	Texture *albedo;
};

#endif // !LAMBERTIAN_MATERIAL_H_DEFINED
