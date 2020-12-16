#ifndef AABB_H_DEFINED
#define AABB_H_DEFINED
#include <limits>
#include "vec.h"
#include <cstdint>
#include "ray.h"
#include <utility>

#define _min(a,b) (a < b ? a : b)
#define _max(a,b) (a < b ? b : a)

template<uint32_t N>
class AABB
{
public:

	constexpr AABB(const vec<N> &givenMin, const vec<N> &givenMax) : min(givenMin), max(givenMax) {}
	constexpr AABB() {}

	[[nodiscard]]
	static AABB<N> united(const AABB<N> &firstBox, const AABB<N> &secondBox)
	{
		vec<N> min = {}, max = {};
		for (uint32_t i = 0; i < N; i++)
		{
			min[i] = _min(firstBox.min[i], secondBox.min[i]);
			max[i] = _max(firstBox.max[i], secondBox.max[i]);
		}
		return AABB<N>(min, max);
	}

	AABB<N> &boundInto(const AABB<N> &other)
	{
		min = min.clampedBy(other.min, other.max);
		max = max.clampedBy(other.min, other.max);
		return *this;
	}

	[[nodiscard]]
	bool hasArea() const
	{
		for (uint32_t i = 0; i < N; i++)
		{
			if (isApproximatively(min[i], max[i], std::numeric_limits<float>::epsilon()))
				return false;
		}
		return true;
	}

	bool hit(const ray &givenRay, float minT, float maxT) const requires (N == 3)
	{
		for (uint32_t a = 0; a < N; a++)
		{
			float invDir = 1.0f / givenRay.direction[a];
			float t0 = (min[a] - givenRay.origin[a]) * invDir;
			float t1 = (max[a] - givenRay.origin[a]) * invDir;
			if (invDir < .0f) std::swap(t0, t1);
			minT = _max(t0, minT);
			maxT = _min(t1, maxT);
			if (maxT <= minT) return false;
		}
		return true;
	};

	float surfaceArea() const requires (N == 3)
	{
		float result = .0f;

		const vec2 front = max.xy() - min.xy();
		result += 2.0f * (front.x() * front.y());

		const vec2 up = vec2(max.x() - min.x(), max.z() - min.z());
		result += 2.0f * (up.x() * up.y());

		const vec2 right = vec2(max.y() - min.y(), max.z() - min.z());
		result += 2.0f * (right.x() * right.y());
		
		return result;
	}

	vec<N> min;
	vec<N> max;

private:

	inline float isApproximatively(float a, float b, float approx)
	{
		return (a + approx) > b && (a - approx) < b;
	}
};

using AABB2 = AABB<2>;
using AABB3 = AABB<3>;

#undef _min
#undef _max

#endif // !AABB_H_DEFINED
