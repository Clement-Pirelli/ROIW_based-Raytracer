#ifndef FLAT_TEXTURE_H_DEFINED
#define FLAT_TEXTURE_H_DEFINED
#include "Texture.h"


class FlatTexture : public Texture
{
public:
	FlatTexture(const vec3 &givenColor) : flatColor(givenColor){}
	~FlatTexture() {}

	virtual vec3 valueAt(float u, float v, const vec3 &point) const override 
	{
		return flatColor;
	}

	vec3 flatColor;
};


#endif