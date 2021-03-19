#include "BvhNode.h"
#include <numeric>
#include <array>

namespace
{
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

	AABB3 getAABB(const std::span<Triangle> triangles, const std::span<size_t> indices)
	{
		if (indices.empty()) return AABB3();

		AABB3 result = triangles[indices.front()].boundingBox();
		
		for (size_t i = 1; i < indices.size(); i++)
		{
			result = AABB3::united(result, triangles[indices[i]].boundingBox());
		}

		return result;
	}
}

BVH::BvhNode::BvhNode(size_t startIndex, float parentSAH, std::span<Triangle> triangles, BvhVector &nodes) {
	const size_t vectorSize = triangles.size();

	std::array<std::vector<size_t>, 3> sortedLists = {};
	BVH::splitPoint bestSplitPoint;
	bestSplitPoint.sah = parentSAH;
	for (size_t i = 0; i < 3; i++)
	{
		const Axis currentAxis = Axis(i);
		auto sortingPredicate = [&](size_t leftIndex, size_t rightIndex)->bool
		{
			const AABB boxLeft = triangles[leftIndex].boundingBox();
			const AABB boxRight = triangles[rightIndex].boundingBox();

			return (boxLeft.min[int(currentAxis)] < boxRight.min[int(currentAxis)]);
		};
		std::vector<size_t>& currentList = sortedLists[i];
		currentList.resize(vectorSize);
		std::iota(currentList.begin(), currentList.end(), 0);
		std::sort(currentList.begin(), currentList.end(), sortingPredicate);
		const Range thisAxisRange = getSortedListAxisRange(triangles[currentList.front()], triangles[currentList.back()], currentAxis);
		const splitPoint current = getSAHSplitPoint(triangles, currentList, currentAxis, thisAxisRange);

		bestSplitPoint = splitPoint::best(current, bestSplitPoint);
	}

	reorder(triangles, sortedLists[bestSplitPoint.axis]);

	if(bestSplitPoint.sah >= parentSAH || triangles.size() <= 2 || bestSplitPoint.index == 0 || bestSplitPoint.index == triangles.size())
	{
		box = getAABB(triangles);
		count = static_cast<int>(triangles.size());
		triangleIndex = startIndex;
	} else 
	{
		split(startIndex, triangles, bestSplitPoint, nodes);

		AABB boxLeft = nodes[leftNode].boundingBox();
		AABB boxRight = nodes[rightNode()].boundingBox();
		box = AABB3::united(boxLeft, boxRight);
	}
}

bool BVH::BvhNode::hit(const std::vector<Triangle> &triangles, const BvhVector &nodes, const ray &givenRay, float minT, float maxT, hitRecord &record) const
{
	if (box.hit(givenRay, minT, maxT))
	{
		if (isLeaf())
		{
			bool hit = false;
			
			hitRecord bestRecord;
			bestRecord.distance = 1000000.0f;

			for(int i = 0; i < count; i++)
			{
				hitRecord currentRecord;
				const bool currentHit = triangles[triangleIndex+i].hit(givenRay, minT, maxT, currentRecord);
				if (currentHit && currentRecord.distance <= bestRecord.distance)
				{
					hit = true;
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

AABB3 BVH::BvhNode::boundingBox() const
{
	return box;
}

BVH::splitPoint BVH::BvhNode::getPercentageSAH(const std::span<Triangle> triangles, const std::span<size_t> indices, const Axis axis, const Range span, float percentage)
{
	const float splitThreshold = span.min + abs(span.max - span.min) * percentage;
	const auto split = std::lower_bound(indices.begin(), indices.end(), splitThreshold, [axis, triangles](size_t a, float b) { return triangles[a].boundingBox().min[(int)axis] < b; });

	const size_t splitIndex = std::distance(indices.begin(), split);
	const std::span<size_t> leftIndices = indices.subspan(0, splitIndex);
	const std::span<size_t> rightIndices = indices.subspan(splitIndex, indices.size() - splitIndex);

	const float sah = getAABB(triangles, leftIndices).surfaceArea() * leftIndices.size() + getAABB(triangles, rightIndices).surfaceArea() * rightIndices.size();

	return splitPoint{ sah, splitIndex, axis };
}

BVH::splitPoint BVH::BvhNode::getSAHSplitPoint(const std::span<Triangle> triangles, const std::span<size_t> indices, const Axis axis, const Range span)
{
	auto getSAH = [&](float percentage) { return getPercentageSAH(triangles, indices, axis, span, percentage); };

	const BVH::splitPoint quarter = getSAH(.25f);
	const BVH::splitPoint middle = getSAH(.5f);
	const BVH::splitPoint threeQuarters = getSAH(.75f);
	return BVH::splitPoint::best(BVH::splitPoint::best(quarter, middle), threeQuarters);
}

Range BVH::BvhNode::getSortedListAxisRange(Triangle &least, Triangle &most, BVH::Axis axis)
{
	const AABB3 boxMin = least.boundingBox();
	const AABB3 boxMax = most.boundingBox();
	return { .min = boxMin.min[int(axis)], .max = boxMax.max[int(axis)] };
}

void BVH::BvhNode::split(size_t startIndex, std::span<Triangle> triangles, splitPoint split, BvhVector &nodes)
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
