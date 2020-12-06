#include "Image.h"

#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace
{
	uint8_t lerp(uint8_t a, uint8_t b, float t)
	{
		return a *(1.0f - t) + b * t;
	}
	color lerp(color first, color second, float t)
	{
		return 
		{ 
			.b = lerp(first.b, second.b, t), 
			.g = lerp(first.g, second.g, t), 
			.r = lerp(first.r, second.r, t),
			.a = lerp(first.a, second.a, t)
		};
	}
}

color Image::atUV(float u, float v) const
{
	const float texelX = u * x;
	const float texelY = v * y;
	const int texelLeft = static_cast<int>(texelX);
	const int texelDown = static_cast<int>(texelY);
	const int texelRight = texelLeft + 1;
	const int texelUp = texelDown + 1;

	const color topLeft = atTexel(texelLeft, texelUp);
	const color topRight = atTexel(texelRight, texelUp);
	const color bottomLeft = atTexel(texelLeft, texelDown);
	const color bottomRight = atTexel(texelRight, texelDown);

	double _;
	const float xDecimal = modf(texelX ,&_);
	const float yDecimal = modf(texelY ,&_);

	return lerp(lerp(topLeft, topRight, xDecimal), lerp(bottomLeft, bottomRight, xDecimal), yDecimal);
}

color Image::atTexel(int texelX, int texelY) const
{
	texelX %= x;
	texelY %= y;
	unsigned char *colorPointer = &data[3 * texelX + 3 * x * texelY];
	return { .b = colorPointer[2], .g = colorPointer[1], .r = colorPointer[0], .a = colorPointer[3] };
}

Image::Image(const char *path)
{
	int nn;
	data = stbi_load(path, &x, &y, &nn, STBI_rgb);
}

Image::~Image()
{
	stbi_image_free(data);
}
