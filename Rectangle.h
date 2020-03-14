#ifndef RECTANGLE_H_DEFINED
#define RECTANGLE_H_DEFINED
#include "Hitable.h"
#include "AABB.h"

class RectangleXY : public Hitable
{
public:
	RectangleXY(){}
	RectangleXY(float givenX0, float givenX1, float givenY0, float givenY1, float givenK, Material *givenMaterial) : x0(givenX0), x1(givenX1), y0(givenY0), y1(givenY1), k(givenK), material(givenMaterial) {}

	virtual bool hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const override
	{
		float distance = (k - givenRay.origin.z) / givenRay.direction.z; 
		if (distance < minT || distance > maxT) return false;
		vec3 hit = givenRay.pointAt(distance);
		if (hit.x < x0 || hit.x > x1 || hit.y < y0 || hit.y > y1) return false;
		
		record.u = (hit.x - x0) / (x1 - x0);
		record.v = (hit.y - y0) / (y1 - y0);

		record.distance = distance;
		record.material = material;
		record.point = hit;
		record.normal = vec3(.0f,.0f, 1.0f);
		return true;
	}


	virtual bool boundingBox(AABB &aabb) const override
	{
		aabb = AABB(vec3(x0,y0, k-.0001f), vec3(x1,y1,k+.0001f));
		return true;
	}

private:
	//k denotes the depth at which the plane lies, x and y describe the limits of the rectangle on the plane
	float x0 = .0f, x1 = .0f, y0 = .0f, y1 = .0f, k = .0f;
	Material *material = nullptr;
};

class RectangleYZ : public Hitable
{
public:
	RectangleYZ() {}
	RectangleYZ(float givenZ0, float givenZ1, float givenY0, float givenY1, float givenK, Material *givenMaterial) : z0(givenZ0), z1(givenZ1), y0(givenY0), y1(givenY1), k(givenK), material(givenMaterial) {}

	virtual bool hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const override
	{
		float distance = (k - givenRay.origin.x) / givenRay.direction.x;
		if (distance < minT || distance > maxT) return false;
		vec3 hit = givenRay.pointAt(distance);
		if (hit.z < z0 || hit.z > z1 || hit.y < y0 || hit.y > y1) return false;

		record.u = (hit.y - y0) / (y1 - y0);
		record.v = (hit.z - z0) / (z1 - z0);

		record.distance = distance;
		record.material = material;
		record.point = hit;
		record.normal = vec3(1.0f, .0f, .0f);
		return true;
	}


	virtual bool boundingBox(AABB &aabb) const override
	{
		aabb = AABB(vec3(k-.0001f, y0, z0), vec3(k + .0001f, y1, z1));
		return true;
	}

private:
	float y0 = .0f, y1 = .0f, z0 = .0f, z1 = .0f, k = .0f;
	Material *material = nullptr;
};

class RectangleXZ : public Hitable
{
public:
	RectangleXZ() {}
	RectangleXZ(float givenX0, float givenX1, float givenZ0, float givenZ1, float givenK, Material *givenMaterial) : x0(givenX0), x1(givenX1), z0(givenZ0), z1(givenZ1), k(givenK), material(givenMaterial) {}

	virtual bool hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const override
	{
		float distance = (k - givenRay.origin.y) / givenRay.direction.y;
		if (distance < minT || distance > maxT) return false;
		vec3 hit = givenRay.pointAt(distance);
		if (hit.x < x0 || hit.x > x1 || hit.z < z0 || hit.z > z1) return false;

		record.u = (hit.x - x0) / (x1 - x0);
		record.v = (hit.z - z0) / (z1 - z0);

		record.distance = distance;
		record.material = material;
		record.point = hit;
		record.normal = vec3(.0f, 1.0f, .0f);
		return true;
	}


	virtual bool boundingBox(AABB &aabb) const override
	{
		aabb = AABB(vec3(x0, k - .0001f, z0), vec3(x1, k + .0001f, z1));
		return true;
	}

private:
	float x0 = .0f, x1 = .0f, z0 = .0f, z1 = .0f, k = .0f;
	Material *material = nullptr;
};



#endif