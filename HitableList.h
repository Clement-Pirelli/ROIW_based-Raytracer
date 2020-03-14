#ifndef HITABLELIST_H_DEFINED
#define HITABLELIST_H_DEFINED
#include "Hitable.h"

template<int Amount>
class HitableList : public Hitable
{
public:
	HitableList() : list{} {} 
	Hitable *list[Amount];
	Hitable *&operator[](int i) { return list[i]; }
	virtual bool hit(const ray& givenRay, float minT, float maxT, hitRecord& record) const override;
	virtual bool boundingBox(AABB &aabb) const override;
};


template<int Amount>
inline bool HitableList<Amount>::hit(const ray& givenRay, float minT, float maxT, hitRecord& record) const
{
	hitRecord tempRecord;
	bool hitAnything = false;
	float closest = maxT;
	for(int i = 0; i < Amount; i++)
	{
		if(list[i]->hit(givenRay, minT, maxT, tempRecord))
		{
			if(tempRecord.distance < closest)
			{
				hitAnything = true;
				closest = tempRecord.distance;
				record = tempRecord;
			}
		}
	}
	return hitAnything;
}

template<int Amount>
inline bool HitableList<Amount>::boundingBox(AABB &aabb) const
{
	if constexpr (Amount < 1) return false;
	AABB tempBox;
	bool firstTrue = list[0]->boundingBox(tempBox);
	
	if (!firstTrue) return false; else aabb = tempBox;

	for(int i = 1; i < Amount; i++)
	{
		if (list[i]->boundingBox(tempBox))
		{
			aabb = AABB::unite(tempBox, aabb);
		}
		else return false;
	}

	return true;
}



#endif