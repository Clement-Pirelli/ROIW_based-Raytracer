#pragma once

#include <math.h>

template<size_t N>
struct vec;

using vec4 = vec<4>;
using vec3 = vec<3>;
using vec2 = vec<2>;

template<size_t N>
struct vec
{
	static_assert(N != 0, "vector must have at least 1 dimension!");

#pragma region macro definitions
#define COMPOUND_VEC_TO_VEC_OPERATOR(op) constexpr vec<N>& operator op(const vec<N> &other)	\
	{																				\
		for (size_t i = 0; i < N; i++)												\
		{																			\
			e[i] op other[i];														\
		}																			\
		return *this;																\
	}

#define COMPOUND_VEC_TO_FLOAT_OPERATOR(op) constexpr vec<N>& operator op(float other)			\
	{																				\
		for (size_t i = 0; i < N; i++)												\
		{																			\
			e[i] op other;															\
		}																			\
		return *this;																\
	}

#define VEC_TO_VEC_OPERATOR(op) [[nodiscard]] constexpr vec<N> operator op(vec<N> other) const\
	{																				\
		vec<N> result = {};															\
		for (size_t i = 0; i < N; i++)												\
		{																			\
			result[i] = e[i] op other[i];											\
		}																			\
		return result;																\
	}

#define VEC_TO_FLOAT_OPERATOR(op) [[nodiscard]] constexpr vec<N> operator op(float other)const\
	{																				\
		vec<N> result = {};															\
		for (size_t i = 0; i < N; i++)												\
		{																			\
			result[i] = e[i] op other;												\
		}																			\
		return result;																\
	}

#define VEC_ELEMENT_GETTER(elementN, name)											\
	[[nodiscard]] const float& name() const requires (N >= elementN){ return e[elementN];};\
	[[nodiscard]] float& name() requires (N >= elementN) { return e[elementN]; };

#pragma endregion

	constexpr vec() = default;

	constexpr vec(const float* elements)
	{
		for (size_t i = 0; i < N; i++)
		{
			e[i] = elements[i];
		}
	}

	[[nodiscard]]
	float operator[](size_t i) const { return e[i]; }
	[[nodiscard]]
	float &operator[](size_t i) { return e[i]; }


	COMPOUND_VEC_TO_VEC_OPERATOR(+= );
	COMPOUND_VEC_TO_VEC_OPERATOR(-= );
	COMPOUND_VEC_TO_VEC_OPERATOR(/= );
	COMPOUND_VEC_TO_VEC_OPERATOR(*= );

	COMPOUND_VEC_TO_FLOAT_OPERATOR(+= );
	COMPOUND_VEC_TO_FLOAT_OPERATOR(-= );
	COMPOUND_VEC_TO_FLOAT_OPERATOR(/= );
	COMPOUND_VEC_TO_FLOAT_OPERATOR(*= );

	VEC_TO_VEC_OPERATOR(+);
	VEC_TO_VEC_OPERATOR(-);
	VEC_TO_VEC_OPERATOR(/ );
	VEC_TO_VEC_OPERATOR(*);

	vec<N> operator-() const {
		vec<N> result;
		for(size_t i= 0; i < N; i++)
		{
			result[i] = -e[i];
		}
		return result;
	}

	VEC_TO_FLOAT_OPERATOR(+);
	VEC_TO_FLOAT_OPERATOR(-);
	VEC_TO_FLOAT_OPERATOR(/ );
	VEC_TO_FLOAT_OPERATOR(*);

	VEC_ELEMENT_GETTER(0, x);
	VEC_ELEMENT_GETTER(1, y);
	VEC_ELEMENT_GETTER(2, z);
	VEC_ELEMENT_GETTER(3, w);

	VEC_ELEMENT_GETTER(0, r);
	VEC_ELEMENT_GETTER(1, g);
	VEC_ELEMENT_GETTER(2, b);
	VEC_ELEMENT_GETTER(3, a);

	[[nodiscard]]
	float length() const
	{
		return sqrtf(squaredLength());
	}

	[[nodiscard]]
	float squaredLength() const
	{
		return dot(*this, *this);
	}

	void normalize()
	{
		*this = *this / length();
	}

	[[nodiscard]]
	vec<N> normalized() const
	{
		return *this / length();
	}

	[[nodiscard]]
	vec<N> clampedBy(vec<N> min, vec<N> max) const
	{
		vec<N> copy = *this;
		for (size_t i = 0; i < N; i++)
		{
			if (copy.e[i] < min.e[i]) copy.e[i] = min.e[i];
			if (copy.e[i] > max.e[i]) copy.e[i] = max.e[i];
		}
		return copy;
	}

	[[nodiscard]]
	vec<N> &saturate()
	{
		for (size_t i = 0; i < N; i++)
		{
			e[i] = e[i] > 1.0f ? 1.0f : e[i];
			e[i] = e[i] < .0f ? .0f : e[i];
		}

		return *this;
	}

	[[nodiscard]]
	static vec<N> lerp(const vec<N> &v1, const vec<N> &v2, float t)
	{
		return v1 * (1.0f - t) + v2 * t;
	}

	[[nodiscard]]
	vec2 xy() const requires (N >= 3)
	{
		return vec2(x(), y());
	};

	[[nodiscard]]
	vec3 xyz() const requires (N >= 4)
	{
		return vec3(x(), y(), z());
	};

	constexpr vec(float givenX, float givenY) requires (N == 2)
	{
		e[0] = givenX;
		e[1] = givenY;
	};

	constexpr vec(float givenX, float givenY, float givenZ) requires (N == 3)
	{
		e[0] = givenX;
		e[1] = givenY;
		e[2] = givenZ;
	};

	constexpr vec(float givenX, float givenY, float givenZ, float givenW) requires (N == 4)
	{
		e[0] = givenX;
		e[1] = givenY;
		e[2] = givenZ;
		e[3] = givenW;
	};

	[[nodiscard]]
	static float dot(const vec<N> &a, const vec<N> &b)
	{
		float result = .0f;
		for (size_t i = 0; i < N; i++)
		{
			result += a[i] * b[i];
		}
		return result;
	}

	[[nodiscard]]
	static vec3 cross(const vec3 &v1, const vec3 &v2) requires (N == 3)
	{
		return vec3({
			v1.y() * v2.z() - v1.z() * v2.y(),
			v1.z() * v2.x() - v1.x() * v2.z(),
			v1.x() * v2.y() - v1.y() * v2.x()
			});
	};

	[[nodiscard]]
	static bool refract(const vec3 &incident, const vec3 &normal, float niOverNt, vec3 &refracted) requires (N == 3)
	{
		vec3 uv = incident.normalized();
		float dt = dot(uv, normal);
		float discriminant = 1.0f - niOverNt * niOverNt * (1.0f - dt * dt);
		if (discriminant > .0f)
		{
			refracted = (uv - normal * dt) * niOverNt - normal * sqrtf(discriminant);
			return true;
		}
		return false;
	};

	[[nodiscard]]
	static vec4 fromPoint(const vec3 &point) requires (N == 4)
	{
		return vec4(point.x(), point.y(), point.z(), 1.0f);
	};

	[[nodiscard]]
	static vec4 fromDirection(const vec3 &direction) requires (N == 4)
	{
		return vec4(direction.x(), direction.y(), direction.z(), .0f);
	};

	[[nodiscard]]
	vec3 rotatedX(float angle) const requires (N == 3)
	{
		return vec3(x(), y() * cosf(angle) - z() * sinf(angle), y() * sinf(angle) + z() * cosf(angle));
	};

	[[nodiscard]]
	vec3 rotatedY(float angle) const requires (N == 3)
	{
		return vec3(x() * cosf(angle) + z() * sinf(angle), y(), -x() * sinf(angle) + z() * cosf(angle));
	};

	[[nodiscard]]
	vec3 rotatedZ(float angle) const requires (N == 3)
	{
		return vec3(x() * cosf(angle) - y() * sinf(angle), x() * sinf(angle) + y() * cosf(angle), z());
	};

	[[nodiscard]]
	static vec3 reflect(const vec3 &incident, const vec3 &normal) requires (N == 3)
	{
		return incident - (normal * dot(incident, normal) * 2.0f);
	};

	[[nodiscard]]
	static constexpr size_t size()
	{
		return N;
	}

	template<size_t N>
	[[nodiscard]]
	static vec<N> min(vec<N> a, vec<N>b)
	{
		vec<N> result;
		for (size_t i = 0; i < N; i++)
		{
			result[i] = a[i] < b[i] ? a[i] : b[i];
		}
		return result;
	}

	template<size_t N>
	[[nodiscard]]
	static vec<N> max(vec<N> a, vec<N>b)
	{
		vec<N> result;
		for (size_t i = 0; i < N; i++)
		{
			result[i] = a[i] > b[i] ? a[i] : b[i];
		}
		return result;
	}

private:
	float e[N] = {};
};




#undef VEC_ELEMENT_GETTER
#undef COMPOUND_VEC_TO_VEC_OPERATOR
#undef COMPOUND_VEC_TO_FLOAT_OPERATOR
#undef VEC_TO_VEC_OPERATOR
#undef VEC_TO_FLOAT_OPERATOR