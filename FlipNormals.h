#ifndef FLIP_NORMALS_H_DEFINED
#define FLIP_NORMALS_H_DEFINED
#include "Hitable.h"
#include "AABB.h"

class FlipNormals : public Hitable
{
public:
	FlipNormals(Hitable *givenHitable) : hitable(givenHitable){}

	virtual bool hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const override
	{
		if(hitable->hit(givenRay, minT, maxT, record))
		{
			record.normal = record.normal * -1.0f;
			return true;
		}
		return false;
	}

	virtual bool boundingBox(AABB &aabb) const override
	{
		return hitable->boundingBox(aabb);
	}

private:
	Hitable *hitable;
};


#endif // !FLIP_NORMALS_H_DEFINED
