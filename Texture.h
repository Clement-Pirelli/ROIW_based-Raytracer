#ifndef TEXTURE_H_DEFINED
#define TEXTURE_H_DEFINED
#include "vec.h"


class Texture
{
public:
	~Texture(){}
	virtual vec3 valueAt(float u, float v, const vec3 &point) const = 0;
};


#endif