#ifndef AABB_H_DEFINED
#define AABB_H_DEFINED
#include <math.h>
#include "vec.h"
#include "ray.h"
#include <utility>

#define _min(a,b) (a < b ? a : b)
#define _max(a,b) (a < b ? b : a)

class AABB
{
public:
	AABB(const vec3 &givenMin, const vec3 &givenMax) : min(givenMin), max(givenMax) {}
	AABB() = default;


	bool hit(const ray &givenRay, float minT, float maxT) const 
	{
		for(int a = 0; a < 3; a++)
		{
			float invDir = 1.0f / givenRay.direction[a];
			float t0 = (min[a] - givenRay.origin[a]) * invDir;
			float t1 = (max[a] - givenRay.origin[a]) * invDir;
			if (invDir < .0f) std::swap(t0, t1);
			minT = _max(t0, minT);
			maxT = _min(t1, maxT);
			if (maxT <= minT) return false;
		}
		return true;
	}

	static AABB unite(const AABB &firstBox, const AABB &secondBox)
	{
		vec3 small = vec3::min(firstBox.min, secondBox.min);
		vec3 big = vec3::max(firstBox.max, secondBox.max);
		return AABB(small, big);
	}

	 vec3 min, max;
};


#endif // !AABB_H_DEFINED
