#pragma once
#include "Hitable.h"
#include "vec.h"
#include "Material.h"

struct Vertex
{
	vec3 position = {};
	float u = .0f, v = .0f;
};

class Triangle
{
public:

	Triangle(Vertex *vertices, Material *mat);

	Vertex vertices[3] = {};
	Material *material = nullptr;

	bool hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const;
	AABB3 boundingBox() const;
};

