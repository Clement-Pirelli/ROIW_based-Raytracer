#ifndef BVH_NODE_H_DEFINED
#define BVH_NODE_H_DEFINED
#include "AABB.h"
#include "randUtils.h"
#include <algorithm>
#include "Logger.h"
#include "Triangle.h"
#include "AlignedAllocator.h"
#include <span>

constexpr size_t maxTrianglesPerLeaf = 5;

class BvhNode;
using BvhVector = std::vector<BvhNode, AlignedAllocator<BvhNode, Alignment::CACHELINE>>;

class BvhNode
{
public:

	BvhNode() = default;
	BvhNode(size_t startIndex, std::span<Triangle> triangles, BvhVector &nodes);



	bool hit(const std::vector<Triangle> &triangles, const BvhVector &nodes, const ray &givenRay, float minT, float maxT, hitRecord &record) const;
	AABB boundingBox() const;

private:

	void split(size_t startIndex, std::span<Triangle> triangles, size_t splitPoint, BvhVector& nodes);

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

//todo
class BVH
{
public:

	BVH build(std::vector<Triangle> &&triangles)
	{
		BVH result;
		result.triangles = std::move(triangles);
		result.nodes.reserve(triangles.size() / maxTrianglesPerLeaf);
		bool done = false;

		while (!done)
		{

		}

		return result;
	}

private:

	BVH() = default;

	BvhVector nodes;
	std::vector<Triangle> triangles;
	std::vector<int> triangleIndices;
};

#endif // !BVH_NODE_H_DEFINED
