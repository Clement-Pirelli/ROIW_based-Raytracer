#pragma once
#include "Hitable.h"
#include "vec.h"
#include "Material.h"

struct Vertex
{
	vec3 position = {};
	float u = .0f, v = .0f;
};

class Triangle :
	public Hitable
{
public:

	Triangle(Vertex *vertices, Material *mat);

	Vertex vertices[3] = {};
	Material *material = nullptr;

	virtual bool hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const override;
	virtual bool boundingBox(AABB &aabb) const override;
};

