#ifndef RAY_H_DEFINED
#define RAY_H_DEFINED
#include "vec3.h"

struct ray
{
	ray() = default;
	ray(const vec3 &givenOrigin, const vec3 &givenDirection) : origin(givenOrigin), direction(givenDirection) {}

	vec3 pointAt(float distance) const
	{
		return origin + direction * distance;
	}

	vec3 origin;
	vec3 direction;
};

#endif // !RAY_H_DEFINED



