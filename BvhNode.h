#ifndef BVH_NODE_H_DEFINED
#define BVH_NODE_H_DEFINED
#include "AABB.h"
#include "Hitable.h"
#include "randUtils.h"
#include <algorithm>
#include "Logger.h"

class BvhNode : public Hitable
{
public:

	BvhNode(){}
	BvhNode(Hitable **hitableList, int hitableAmount)
	{
		enum class Axis
		{
			x = 0,
			y = 1,
			z = 2
		};
		Axis axis = Axis(int(3.0f*drand48()));
		
		//sort the list depending on the random axis
		std::sort(
			hitableList,
			hitableList + hitableAmount,
			[=](Hitable *left, Hitable *right)->bool
			{
				AABB boxLeft, boxRight;
				if (!left->boundingBox(boxLeft) || !right->boundingBox(boxRight)) 
					Logger::LogError("Couldn't create bounding box in BVH node constructor!", __LINE__, __FILE__);

				return (boxLeft.min.e[int(axis)] < boxRight.min.e[int(axis)]);
			}
		);

		if(hitableAmount == 1)
		{
			left = right = hitableList[0];
		} else 
		if(hitableAmount == 2)
		{
			left = hitableList[0];
			right = hitableList[1];
		} else 
		{
			int halfAmount = hitableAmount / 2;
			left = new BvhNode(hitableList, halfAmount);
			right = new BvhNode(hitableList + halfAmount, hitableAmount - halfAmount);
		}

		AABB boxLeft, boxRight;
		if (!left->boundingBox(boxLeft) || !right->boundingBox(boxRight))
			Logger::LogError("Couldn't create bounding box in BVH node constructor!", __LINE__, __FILE__);
		box = AABB::unite(boxLeft, boxRight);
	}

	virtual bool hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const override
	{
		if(box.hit(givenRay, minT, maxT))
		{
			hitRecord leftRecord, rightRecord;
			bool hitLeft = left->hit(givenRay, minT, maxT, leftRecord);
			bool hitRight = right->hit(givenRay, minT, maxT, rightRecord);

			if(hitLeft && hitRight)
			{
				if (leftRecord.distance < rightRecord.distance) record = leftRecord; else record = rightRecord;
				return true;
			} else if(hitLeft)
			{
				record = leftRecord;
				return true;
			} else if(hitRight)
			{
				record = rightRecord;
				return true;
			} else 
				return false;
		} else 
		return false;
	}

	virtual bool boundingBox(AABB &aabb) const override
	{
		aabb = box;
		return true;
	}


	Hitable *left = nullptr, *right = nullptr;
	AABB box;
};

#endif // !DIELECTRIC_MATERIAL_H_DEFINED
