#ifndef CAMERA_H_DEFINED
#define CAMERA_H_DEFINED
#include "ray.h"
#include <math.h>
#include "vec3.h"
#include "randUtils.h"

constexpr float deg2rad = 3.1415926524f / 180.0f;

class Camera
{
public:
	Camera(){}
	Camera(const vec3 &lookFrom, const vec3 &lookAt, const vec3 &viewUp, float verticalFOV, float aspectRatio, float aperture, float focusDistance)
	{
		lensRadius = aperture * .5f;
		

		float theta = verticalFOV * deg2rad;
		float halfHeight = tanf(theta * .5f);
		float halfWidth = aspectRatio * halfHeight;

		origin = lookFrom;
		
		w = (lookFrom - lookAt).normalized();
		u = vec3::cross(viewUp, w).normalized();
		v = vec3::cross(w, u);
		

		bottomLeftCorner = origin - u * halfWidth*focusDistance - v * halfHeight*focusDistance - w*focusDistance;
		horizontal = u * 2.0f * focusDistance * halfWidth;
		vertical = v * 2.0f * focusDistance * halfHeight;
	}

	ray getRay(float s, float t) const
	{
		vec3 random = randInUnitDisk()*lensRadius;
		vec3 offset = u * random.x + v * random.y;
		return ray(origin + offset, bottomLeftCorner + horizontal * s + vertical * t - origin - offset);
	}

	vec3 horizontal;
	vec3 vertical;
	vec3 origin;
	vec3 bottomLeftCorner;
	vec3 u, v, w; //bases
	float lensRadius = .0f;
};

#endif