#include "BvhNode.h"
#include <numeric>

namespace
{
	enum Axis
	{
		x = 0,
		y = 1,
		z = 2
	};

	struct Range 
	{ 
		float min{};  
		float max{};

		float span() const { return max - min; }
	};

	template <class T>
	void reorder(std::span<T> &span, std::vector<size_t> &indices)
	{
		// for all elements to put in place
		for (size_t i = 0; i < span.size(); ++i)
		{
			// while vOrder[i] is not yet in place 
			// every swap places at least one element in it's proper place
			while (indices[i] != indices[indices[i]])
			{
				std::swap(span[indices[i]], span[indices[indices[i]]]);
				std::swap(indices[i], indices[indices[i]]);
			}
		}
	}

	size_t getSAHSplitPoint(const std::span<Triangle> triangles, const Axis axis, const Range span)
	{
		//todo
		/*struct splitPoint
		{
			float sah;
			size_t index;
		};

		auto getSAH = [&](float percentage) 
		{
			const float splitThreshold = span.min + span.max * percentage;

			
		};*/



		return triangles.size() / 2;
	}

	Range getSortedListAxisRange(Triangle &least, Triangle &most, Axis axis)
	{
		const AABB boxMin = least.boundingBox();
		const AABB boxMax = most.boundingBox();
		return { .min = boxMax.max[int(axis)], .max = boxMin.min[int(axis)] };
	}

	struct MaxAxisResult
	{
		Axis axis{};
		Range range{};
	};

	MaxAxisResult chooseMaxAxis(std::span<Triangle> triangles, std::vector<size_t> *sortedLists)
	{
		const size_t vectorSize = triangles.size();
		MaxAxisResult result{};
		for (size_t i = 0; i < 3; i++)
		{
			const Axis currentAxis = Axis(i);
			auto sortingPredicate = [&](int leftIndex, int rightIndex)->bool
			{
				const AABB boxLeft = triangles[leftIndex].boundingBox();
				const AABB boxRight = triangles[rightIndex].boundingBox();

				return (boxLeft.min[int(currentAxis)] < boxRight.min[int(currentAxis)]);
			};
			std::vector<size_t> &currentList = sortedLists[i];
			currentList.resize(vectorSize);
			std::iota(currentList.begin(), currentList.end(), 0);
			std::sort(currentList.begin(), currentList.end(), sortingPredicate);
			const Range thisAxisRange = getSortedListAxisRange(triangles[currentList.front()], triangles[currentList.back()], currentAxis);

			if (thisAxisRange.span() >= result.range.span()) { result.range = thisAxisRange; result.axis = currentAxis; }
		}

		return result;
	}
}

BvhNode::BvhNode(size_t startIndex, std::span<Triangle> triangles, BvhVector &nodes) {
	const size_t vectorSize = triangles.size();

	if(vectorSize <= maxTrianglesPerLeaf)
	{
		box = triangles[0].boundingBox();
		for(size_t i = 1; i < triangles.size(); i++)
		{
			box= AABB::unite(box, triangles[i].boundingBox());
		}
		count = triangles.size();
		triangleIndex = startIndex;
	}
	else
	{
		std::vector<size_t> sortedLists[3] = {};

		const MaxAxisResult maxAxisResult = chooseMaxAxis(triangles, sortedLists);

		reorder(triangles, sortedLists[(int)maxAxisResult.axis]);

		size_t sahSplitPoint = getSAHSplitPoint(triangles, maxAxisResult.axis, maxAxisResult.range);
		split(startIndex, triangles, sahSplitPoint, nodes);

		AABB boxLeft = nodes[leftNode].boundingBox();
		AABB boxRight = nodes[rightNode()].boundingBox();
		box = AABB::unite(boxLeft, boxRight);
	}
}

bool BvhNode::hit(const std::vector<Triangle> &triangles, const BvhVector &nodes, const ray &givenRay, float minT, float maxT, hitRecord &record) const
{
	if (box.hit(givenRay, minT, maxT))
	{
		if (isLeaf())
		{
			bool hit = false;
			
			hitRecord records[maxTrianglesPerLeaf];
			int resultIndex = 0;
			float min = 1000000.0f;

			for(int i = 0; i < count; i++)
			{
				const bool currentHit = triangles[triangleIndex+i].hit(givenRay, minT, maxT, records[i]);
				hit |= currentHit;
				if (currentHit && records[i].distance <= min)
				{
					min = records[i].distance;
					resultIndex = i;
				}
			}
			record = records[resultIndex];
			return hit;
		}
		else
		{
			hitRecord leftRecord, rightRecord;
			bool hitLeft = nodes[leftNode].hit(triangles, nodes, givenRay, minT, maxT, leftRecord);
			bool hitRight = nodes[rightNode()].hit(triangles, nodes, givenRay, minT, maxT, rightRecord);

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

AABB BvhNode::boundingBox() const
{
	return box;
}

void BvhNode::split(size_t startIndex, std::span<Triangle> triangles, size_t splitPoint, BvhVector &nodes)
{
	//the left node will be after the last node of the vector
	leftNode = nodes.size();
	//emplace an empty node for now. This will be where the left node is, but we want the left and right node to be contiguous
	//if we pushed back the left node now, it might push back another node. This would mean we'd never be able to find our right node!
	nodes.push_back(BvhNode());
	nodes.push_back(BvhNode());
	
	size_t leftTrianglesCount = triangles.size() - splitPoint;
	nodes[rightNode()] = BvhNode(startIndex, triangles.subspan(0, splitPoint), nodes);
	nodes[leftNode] = BvhNode(startIndex+splitPoint, triangles.subspan(splitPoint, leftTrianglesCount), nodes);
}
