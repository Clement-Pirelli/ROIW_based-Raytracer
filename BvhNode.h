#ifndef BVH_NODE_H_DEFINED
#define BVH_NODE_H_DEFINED
#include "AABB.h"
#include "randUtils.h"
#include <algorithm>
#include "Logger.h"
#include "Triangle.h"
#include "AlignedAllocator.h"
#include <span>
#include "Material.h"

struct Range
{
	float min{};
	float max{};
};

class BVH
{
public:

	BVH(std::vector<Triangle> &&givenTriangles, Material *givenMaterial) : triangles(std::move(givenTriangles))
	{
		nodes.reserve(triangles.size() / 3);
		nodes.resize(2);
		nodes.front() = BvhNode(0U, std::numeric_limits<float>::max(), triangles, nodes);
		material = givenMaterial;
	}

	size_t triangleCount() const { return triangles.size(); }

	[[nodiscard]]
	bool hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const
	{
		const bool result = nodes.front().hit(triangles, nodes, givenRay, minT, maxT, record);
		record.material = material;
		return result;
	}

	[[nodiscard]]
	AABB3 boundingBox() const 
	{
		return nodes.front().boundingBox();
	}

private:

	BVH() = default;
	class BvhNode;
	using BvhVector = std::vector<BvhNode, AlignedAllocator<BvhNode, Alignment::CACHELINE>>;

	enum Axis
	{
		x = 0,
		y = 1,
		z = 2
	};

	struct splitPoint
	{
		float sah{};
		size_t index{};
		Axis axis{};

		static splitPoint best(splitPoint a, splitPoint b) { return a.sah < b.sah ? a : b; }
	};

	class BvhNode
	{
	public:

		BvhNode() = default;
		BvhNode(size_t startIndex, float parentSAH, std::span<Triangle> triangles, BvhVector &nodes);

		[[nodiscard]]
		bool hit(const std::vector<Triangle> &triangles, const BvhVector &nodes, const ray &givenRay, float minT, float maxT, hitRecord &record) const;
		AABB3 boundingBox() const;

	private:
		
		static BVH::splitPoint getPercentageSAH(const std::span<Triangle> triangles, const std::span<size_t> indices, const Axis axis, const Range span, float percentage);
		static BVH::splitPoint getSAHSplitPoint(const std::span<Triangle> triangles, const std::span<size_t> indices, const Axis axis, const Range span);
		static Range getSortedListAxisRange(Triangle &least, Triangle &most, BVH::Axis axis);
		
		void split(size_t startIndex, std::span<Triangle> triangles, splitPoint split, BvhVector &nodes);

		AABB3 box;
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

	BvhVector nodes;
	std::vector<Triangle> triangles;
	std::vector<int> triangleIndices;
	Material *material = nullptr;
};

#endif // !BVH_NODE_H_DEFINED
