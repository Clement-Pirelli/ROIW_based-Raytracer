#ifndef BVH_NODE_H_DEFINED
#define BVH_NODE_H_DEFINED
#include "AABB.h"
#include "Hitable.h"
#include "randUtils.h"
#include <algorithm>
#include "Logger.h"

class BvhNode : public Hitable
{
private:
	enum class Axis
	{
		x = 0,
		y = 1,
		z = 2,
		NONE
	};
public:

	BvhNode(){}
	BvhNode(Hitable **hitableList, int hitableAmount, Axis sortedAxis = Axis::NONE)
	{
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
			Hitable **sortedLists[3] = {};
			
			Axis axis = Axis::x;
			
			auto sortingPredicate = [&](Hitable *left, Hitable *right)->bool
			{
				AABB boxLeft, boxRight;
				if (!left->boundingBox(boxLeft) || !right->boundingBox(boxRight))
					Logger::LogError("Couldn't create bounding box in BVH node constructor!", __LINE__, __FILE__);

				return (boxLeft.min[int(axis)] < boxRight.min[int(axis)]);
			};

			Axis maxAxis = sortedAxis;
			
			auto getSortedListAxisSpan = [](Hitable *min, Hitable *max, Axis axis) ->float
			{
				AABB boxMin, boxMax;
				if (!min->boundingBox(boxMin) || !max->boundingBox(boxMax))
					Logger::LogError("Couldn't create bounding box in BVH node constructor!", __LINE__, __FILE__);
				return boxMax.max[int(axis)] - boxMin.min[int(axis)];
			};

			float maxAxisSpan = getSortedListAxisSpan(hitableList[0], hitableList[hitableAmount - 1], sortedAxis);

			for (int i = 0; i < 3; i++)
			{
				axis = Axis(i);
				if (axis == sortedAxis) continue;
				sortedLists[i] = new Hitable * [hitableAmount];
				std::copy(hitableList, hitableList + hitableAmount, sortedLists[i]);
				std::sort(sortedLists[i], sortedLists[i] + hitableAmount, sortingPredicate);
				float thisAxisSpan = getSortedListAxisSpan(sortedLists[i][0], sortedLists[i][hitableAmount - 1], axis);

				if (thisAxisSpan > maxAxisSpan) { maxAxisSpan = thisAxisSpan; maxAxis = axis; }
			}

			int halfAmount = hitableAmount / 2;
			left = new BvhNode(sortedLists[int(maxAxis)], halfAmount);
			right = new BvhNode(sortedLists[int(maxAxis)] + halfAmount, hitableAmount - halfAmount);
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

private:

	Hitable *left = nullptr, *right = nullptr;
	AABB box;
};

#endif // !BVH_NODE_H_DEFINED
