#ifndef CONSTANT_MEDIUM_H_DEFINED
#define CONSTANT_MEDIUM_H_DEFINED
#include "Hitable.h"
#include "Material.h"
#include "Volume.h"
#include "Texture.h"

#define _min(a,b) (a < b ? a : b)
#define _max(a,b) (a < b ? b : a)

class ConstantMedium : public Hitable
{
public:

	ConstantMedium(Hitable *givenBoundaries, float givenDensity, Texture *albedo) : density(givenDensity), boundaries(givenBoundaries)
	{
		phaseFunction = new Volume(albedo);
	}


	virtual bool hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const override
	{
		hitRecord firstHit, secondHit;
		if(boundaries->hit(givenRay, -100000.0f, 100000.0f, firstHit))
		{
			if(boundaries->hit(givenRay, firstHit.distance+.0001f, 10000.0f, secondHit))
			{
				firstHit.distance = _max(firstHit.distance, minT);
				secondHit.distance = _min(secondHit.distance, maxT);
				if (firstHit.distance >= secondHit.distance) return false;
				firstHit.distance = _max(firstHit.distance, .0f);
				float distanceInsideBoundary = (givenRay.direction * (secondHit.distance - firstHit.distance)).length();
				float hitDistance = -(1.0f / density) * logf(drand48());
				if(hitDistance < distanceInsideBoundary)
				{
					record.distance = firstHit.distance + hitDistance / givenRay.direction.length();
					record.point = givenRay.pointAt(record.distance);
					record.normal = vec3(1.0f, .0f, .0f); //arbitrary : the material scatters at a random angle
					record.material = phaseFunction;
					return true;
				}
			}
		}
		return false;
	}

	virtual bool boundingBox(AABB &aabb) const override
	{
		return boundaries->boundingBox(aabb);
	}

private:

	float density;
	Hitable *boundaries;
	Material *phaseFunction;
};


#endif