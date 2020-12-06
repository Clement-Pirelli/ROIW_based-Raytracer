#include "ImageTexture.h"
#include "color.h"

ImageTexture::ImageTexture(const char *path) : image(path)
{
}

vec3 ImageTexture::valueAt(float u, float v, const vec3 &point) const
{
	color rgb = image.atUV(u,v);

	vec3 color = vec3(
		int(rgb.r) / 255.0f,
		int(rgb.g) / 255.0f,
		int(rgb.b) / 255.0f
	);

	return color;
}
