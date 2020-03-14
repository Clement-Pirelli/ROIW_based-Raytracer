#ifndef METAL_MATERIAL_H_DEFINED
#define METAL_MATERIAL_H_DEFINED
#include "Material.h"
#include "randUtils.h"

class Metal : public Material
{
public:
	Metal(const vec3 &givenAlbedo, float givenFuzziness) : albedo(givenAlbedo), fuzziness(givenFuzziness) {}

	virtual bool scatter(const ray &inRay, const hitRecord &record, vec3 &attenuation, ray &scatteredRay) const override
	{
		vec3 reflected = vec3::reflect(inRay.direction, record.normal);
		scatteredRay = ray(record.point, reflected + randInUnitSphere()* fuzziness);
		attenuation = albedo;
		return vec3::dot(scatteredRay.direction, record.normal) > .0f;
	}


	vec3 albedo;
	float fuzziness;
};

#endif // !METAL_MATERIAL_H_DEFINED
