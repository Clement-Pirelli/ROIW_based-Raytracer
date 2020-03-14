#include "Sphere.h"
#include <math.h>
#include <random>

bool Sphere::hit(const ray& givenRay, float minT, float maxT, hitRecord& record) const
{
	vec3 originToCenter = givenRay.origin - center;
	float a = vec3::dot(givenRay.direction, givenRay.direction);
	float b = vec3::dot(originToCenter, givenRay.direction);
	float c = vec3::dot(originToCenter, originToCenter) - radius * radius;
	float discriminant = b * b - a * c;
	if (discriminant > .0f)
	{
		float t1 = (-b - sqrtf(discriminant))/a;
		float t2 = (-b + sqrtf(discriminant)) / a;
		if(t1 > minT && t1 < maxT)
		{
			record.distance = t1;
			record.point = givenRay.pointAt(record.distance);
			record.exitPoint = givenRay.pointAt(t2);
			record.normal = (record.point - center) / radius;
			record.material = material;
			return true;
		}
		if (t2 > minT&& t2 < maxT)
		{
			record.distance = t2;
			record.point = givenRay.pointAt(record.distance);
			record.exitPoint = givenRay.pointAt(t1);
			record.normal = (record.point - center) / radius;
			record.material = material;
			return true;
		}
	}

	return false;
}

bool Sphere::boundingBox(AABB &aabb) const
{
	aabb = AABB(center - vec3(radius, radius, radius), center + vec3(radius, radius, radius));
	return true;
}
