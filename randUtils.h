#ifndef RANDUTILS_H_DEFINED
#define RANDUTILS_H_DEFINED

#include "vec.h"
#include <cmath>
#include <random>

inline float drand48() {
	return static_cast<float>(rand()) / RAND_MAX;
}

//by @criver#8473 on discord
// maps [0,1]^2 rect to cosine weighted hemisphere
inline vec3 mapRectToCosineHemisphere(const vec3 &n, float u, float v)
{
    // create tnb:
    //http://jcgt.org/published/0006/01/01/paper.pdf
    float signZ = (n.z() >= 0.0f) ? 1.0f : -1.0f;     //do not use sign(nor.z), it can produce 0.0
    float a = -1.0f / (signZ + n.z());
    float b = n.x() * n.y() * a;
    vec3 b1 = vec3(1.0f + signZ * n.x() * n.x() * a, signZ * b, -signZ * n.x());
    vec3 b2 = vec3(b, signZ + n.y() * n.y() * a, -n.y());

    // remap uv to cosine distributed points on the hemisphere around n
    float phi = 2.0f * 3.14f * u;
    float cosTheta = sqrt(v);
    float sinTheta = sqrt(1.0f - v);
    return ((b1 * cosf(phi) + b2 * sinf(phi)) * cosTheta + n * sinTheta).normalized();
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


