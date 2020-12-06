#include "ImageTexture.h"


ImageTexture::ImageTexture(const char *path) : image(path)
{
}

vec3 ImageTexture::valueAt(float u, float v, const vec3 &point) const
{
	unsigned char *rgb = image.atUV(u,v);

	vec3 color = vec3(
		int(rgb[0]) / 255.0f,
		int(rgb[1]) / 255.0f,
		int(rgb[2]) / 255.0f
	);

	return color;
}
