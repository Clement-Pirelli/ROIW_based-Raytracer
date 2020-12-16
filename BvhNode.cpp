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

	AABB3 getAABB(const std::span<Triangle> triangles)
	{
		if (triangles.empty()) return AABB3();

		AABB3 result = triangles[0].boundingBox();
		
		for (size_t i = 1; i < triangles.size(); i++)
		{
			result = AABB3::united(result, triangles[i].boundingBox());
		}

		return result;
	}

	splitPoint getPercentageSAH(const std::span<Triangle> triangles, const Axis axis, const Range span, float percentage)
	{
		const float splitThreshold = span.min + abs(span.max-span.min) * percentage;
		auto split = std::lower_bound(triangles.begin(), triangles.end(), splitThreshold, [axis](const Triangle &a, float b) { return a.boundingBox().min[(int)axis] < b; });

		const size_t splitIndex = std::distance(triangles.begin(), split);
		const std::span<Triangle> leftTriangles = triangles.subspan(0, splitIndex);
		const std::span<Triangle> rightTriangles = triangles.subspan(splitIndex, triangles.size() - splitIndex);

		const float sah = getAABB(leftTriangles).surfaceArea() * leftTriangles.size() + getAABB(rightTriangles).surfaceArea() * rightTriangles.size();
		
		return splitPoint{ sah, splitIndex };
	}

	splitPoint getSAHSplitPoint(const std::span<Triangle> triangles, const Axis axis, const Range span)
	{
		auto getSAH = [&](float percentage) { return getPercentageSAH(triangles, axis, span, percentage); };
		auto bestSplit = [](splitPoint a, splitPoint b) { return a.sah < b.sah ? a : b; };

		const splitPoint quarter = getSAH(.25f);
		const splitPoint middle = getSAH(.5f);
		const splitPoint threeQuarters = getSAH(.75f);
		return bestSplit(bestSplit(quarter, middle), threeQuarters);
	}

	Range getSortedListAxisRange(Triangle &least, Triangle &most, Axis axis)
	{
		const AABB3 boxMin = least.boundingBox();
		const AABB3 boxMax = most.boundingBox();
		return { .min = boxMin.min[int(axis)], .max = boxMax.max[int(axis)] };
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

BvhNode::BvhNode(size_t startIndex, float parentSAH, std::span<Triangle> triangles, BvhVector &nodes) {
	const size_t vectorSize = triangles.size();

	std::vector<size_t> sortedLists[3] = {};

	const MaxAxisResult maxAxisResult = chooseMaxAxis(triangles, sortedLists);

	reorder(triangles, sortedLists[(int)maxAxisResult.axis]);

	splitPoint sahSplitPoint = getSAHSplitPoint(triangles, maxAxisResult.axis, maxAxisResult.range);

	if(sahSplitPoint.sah >= parentSAH || triangles.size() <= 2)
	{
		box = getAABB(triangles);
		count = triangles.size();
		triangleIndex = startIndex;
	} else 
	{
		split(startIndex, triangles, sahSplitPoint, nodes);

		AABB boxLeft = nodes[leftNode].boundingBox();
		AABB boxRight = nodes[rightNode()].boundingBox();
		box = AABB3::united(boxLeft, boxRight);
	}
}

bool BvhNode::hit(const std::vector<Triangle> &triangles, const BvhVector &nodes, const ray &givenRay, float minT, float maxT, hitRecord &record) const
{
	if (box.hit(givenRay, minT, maxT))
	{
		if (isLeaf())
		{
			bool hit = false;
			
			hitRecord bestRecord;
			int resultIndex = 0;
			float min = 1000000.0f;

			for(int i = 0; i < count; i++)
			{
				hitRecord currentRecord;
				const bool currentHit = triangles[triangleIndex+i].hit(givenRay, minT, maxT, currentRecord);
				hit |= currentHit;
				if (currentHit && currentRecord.distance <= bestRecord.distance)
				{
					bestRecord = currentRecord;
				}
			}
			record = bestRecord;
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

AABB3 BvhNode::boundingBox() const
{
	return box;
}

void BvhNode::split(size_t startIndex, std::span<Triangle> triangles, splitPoint split, BvhVector &nodes)
{
	//the left node will be after the last node of the vector
	leftNode = nodes.size();
	//emplace an empty node for now. This will be where the left node is, but we want the left and right node to be contiguous
	//if we pushed back the left node now, it might push back another node. This would mean we'd never be able to find our right node!
	nodes.push_back(BvhNode());
	nodes.push_back(BvhNode());
	
	size_t leftTrianglesCount = triangles.size() - split.index;
	nodes[rightNode()] = BvhNode(startIndex, split.sah, triangles.subspan(0, split.index), nodes);
	nodes[leftNode] = BvhNode(startIndex+ split.index, split.sah, triangles.subspan(split.index, leftTrianglesCount), nodes);
}
