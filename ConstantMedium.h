#ifndef CONSTANT_MEDIUM_H_DEFINED
#define CONSTANT_MEDIUM_H_DEFINED
#include "Hitable.h"
#include "Material.h"
#include "Volume.h"
#include "Texture.h"
#include "BvhNode.h"

#define _min(a,b) (a < b ? a : b)
#define _max(a,b) (a < b ? b : a)

class ConstantMedium
{
public:

	ConstantMedium(BVH &&givenBoundaries, float givenDensity, Texture *albedo) : 
		density(givenDensity), 
		boundaries(std::move(givenBoundaries)), 
		volume(std::make_unique<Volume>(albedo))
	{}


	bool hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const
	{
		hitRecord firstHit, secondHit;
		if(boundaries.hit(givenRay, -100000.0f, 100000.0f, firstHit))
		{
			if(boundaries.hit(givenRay, firstHit.distance+.0001f, 10000.0f, secondHit))
			{
				firstHit.distance = _max(firstHit.distance, minT);
				secondHit.distance = _min(secondHit.distance, maxT);
				if (firstHit.distance >= secondHit.distance) return false;
				firstHit.distance = _max(firstHit.distance, .0f);
				float distanceInsideBoundary = (givenRay.direction * (secondHit.distance - firstHit.distance)).length();
				float hitDistance = -(1.0f / density) * logf(drand48());
				if(hitDistance < distanceInsideBoundary)
				{
					record.distance = firstHit.distance + hitDistance;
					record.point = givenRay.pointAt(record.distance);
					record.normal = vec3(1.0f, .0f, .0f); //arbitrary : the material scatters at a random angle
					record.material = volume.get();
					return true;
				}
			}
		}
		return false;
	}

	AABB3 boundingBox() const
	{
		return boundaries.boundingBox();
	}

	BVH boundaries;

private:

	float density;
	
	std::unique_ptr<Volume> volume;
};

#undef _min
#undef _max

#endif