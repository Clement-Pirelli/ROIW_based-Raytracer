#ifndef RANDUTILS_H_DEFINED
#define RANDUTILS_H_DEFINED

#include "vec3.h"
#include <cmath>
#include <random>

inline float drand48() {
	return static_cast<float>(rand()) / RAND_MAX;
}

inline vec3 randInUnitSphere()
{
	vec3 point;
	do
	{
		point = vec3(drand48(), drand48(), drand48()) * 2.0f - vec3(1.0f, 1.0f, 1.0f);
	} while (point.squaredLength() >= 1.0f);
	return point;
}

inline vec3 randInUnitDisk()
{
	vec3 point;
	do
	{
		point = vec3(drand48(), drand48(), .0f) * 2.0f - vec3(1.0f, 1.0f, .0f);
	} while (point.squaredLength() >= 1.0f);
	return point;
}

inline vec3 randVec3()
{
	return vec3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48());
}

#endif


