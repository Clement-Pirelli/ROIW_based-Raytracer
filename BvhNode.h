#ifndef BVH_NODE_H_DEFINED
#define BVH_NODE_H_DEFINED
#include "AABB.h"
#include "randUtils.h"
#include <algorithm>
#include "Logger.h"
#include "Triangle.h"
#include "AlignedAllocator.h"



class BvhNode
{

	enum class Axis
	{
		x = 0,
		y = 1,
		z = 2,
		NONE
	};

public:

	using BvhVector = std::vector<BvhNode, AlignedAllocator<BvhNode, Alignment::CACHELINE>>;

	BvhNode(){}
	BvhNode(const std::vector<Triangle> &triangles, std::vector<int> triangleIndices, BvhVector &nodes, Axis sortedAxis = Axis::NONE)
	{
		size_t vectorSize = triangleIndices.size();
		if (vectorSize == 1)
		{
			triangleIndex = triangleIndices[0];
			if (!triangles[triangleIndex].boundingBox(box))
				LOGERROR_LINEFILE("Couldn't create bounding box in BVH node constructor!");
			count = 1;
			return;
		} else 
		if(vectorSize == 2)
		{
			emplaceNodes(nodes, triangles, triangleIndices, sortedAxis);
		} else 
		{
			std::vector<int> sortedLists[3] = {};
			
			Axis axis = Axis::x;
			
			auto sortingPredicate = [&](int leftIndex, int rightIndex)->bool
			{
				AABB boxLeft, boxRight;
				if (!triangles[leftIndex].boundingBox(boxLeft) || !triangles[rightIndex].boundingBox(boxRight))
					LOGERROR_LINEFILE("Couldn't create bounding box in BVH node constructor!");

				return (boxLeft.min[int(axis)] < boxRight.min[int(axis)]);
			};

			Axis maxAxis = sortedAxis;
			if(maxAxis != Axis::NONE)
			{
				sortedLists[int(maxAxis)] = triangleIndices;
			}
			
			auto getSortedListAxisSpan = [&](int min, int max, Axis axis) ->float
			{
				AABB boxMin, boxMax;
				if (!triangles[min].boundingBox(boxMin) || !triangles[max].boundingBox(boxMax))
					LOGERROR_LINEFILE("Couldn't create bounding box in BVH node constructor!");
				return boxMax.max[int(axis)] - boxMin.min[int(axis)];
			};

			float maxAxisSpan = getSortedListAxisSpan(triangleIndices[0], triangleIndices[vectorSize - 1], sortedAxis);

			for (int i = 0; i < 3; i++)
			{
				axis = Axis(i);
				if (axis == sortedAxis) continue;
				sortedLists[i].resize(vectorSize);
				std::copy(triangleIndices.begin(), triangleIndices.end(), sortedLists[i].data());
				std::sort(sortedLists[i].begin(), sortedLists[i].end(), sortingPredicate);
				float thisAxisSpan = getSortedListAxisSpan(sortedLists[i][0], sortedLists[i][vectorSize - 1], axis);

				if (thisAxisSpan > maxAxisSpan) { maxAxisSpan = thisAxisSpan; maxAxis = axis; }
			}
			if (maxAxis == Axis::NONE) maxAxis = Axis::x;
			emplaceNodes(nodes, triangles, sortedLists[int(maxAxis)], maxAxis);
		}

		AABB boxLeft, boxRight;
		if (!nodes[leftNode].boundingBox(boxLeft) || !nodes[rightNode()].boundingBox(boxRight))
			Logger::LogError("Couldn't create bounding box in BVH node constructor!", __LINE__, __FILE__);
		box = AABB::unite(boxLeft, boxRight);
	}



	bool hit(const std::vector<Triangle> &triangles, const BvhVector &nodes, const ray &givenRay, float minT, float maxT, hitRecord &record) const
	{
		if (box.hit(givenRay, minT, maxT))
		{
			if (isLeaf())
			{
				return triangles[triangleIndex].hit(givenRay, minT, maxT, record);
			}
			else
			{
				hitRecord leftRecord, rightRecord;
				bool hitLeft = nodes[leftNode].hit(triangles, nodes, givenRay, minT, maxT, leftRecord);
				bool hitRight = nodes[rightNode()].hit(triangles, nodes, givenRay, minT, maxT, rightRecord);;

				if (hitLeft && hitRight)
				{
					if (leftRecord.distance < rightRecord.distance) record = leftRecord; else record = rightRecord;
					return true;
				}
				else if (hitLeft)
				{
					record = leftRecord;
					return true;
				}
				else if (hitRight)
				{
					record = rightRecord;
					return true;
				}
				else
					return false;
			}
		}
		
		return false;
	}

	bool boundingBox(AABB &aabb) const
	{
		aabb = box;
		return true;
	}




private:

	void emplaceNodes(BvhVector &nodes, const std::vector<Triangle> &triangles, const std::vector<int> &sortedList, Axis maxAxis)
	{
		size_t halfAmount = sortedList.size() / 2;
		//the left node will be after the last node of the vector
		leftNode = nodes.size();
		//emplace an empty node for now. This will be where the left node is, but we want the left and right node to be contiguous
		//if we pushed back the left node now, it might push back another node. This would mean we'd never be able to find our right node!
		nodes.push_back(BvhNode());
		nodes.push_back(BvhNode());
		//todo: fix this
		nodes[rightNode()] = BvhNode(triangles, std::vector<int>(sortedList.begin() + halfAmount, sortedList.end()), nodes, maxAxis);
		nodes[leftNode] = BvhNode(triangles, std::vector<int>(sortedList.begin(), sortedList.begin() + halfAmount), nodes, maxAxis);
	}


	AABB box;
	//credit to Zuen#5394 on discord for this bvh node layout
	union {
		size_t leftNode = ~0U;  // index of left child if the current Node is not a leaf Node
		size_t triangleIndex; // index of the triangle if the current Node is a leaf Node
	};
	int count = 0;

	inline bool isLeaf() const {
		return count > 0;
	}

	inline size_t rightNode() const {
		return leftNode + 1;
	}

};

#endif // !BVH_NODE_H_DEFINED
