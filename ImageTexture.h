#pragma once
#include "Texture.h"
#include "ImageLoader.h"


class ImageTexture : public Texture
{
public:
	ImageTexture(const char *path);
	~ImageTexture();

	virtual vec3 valueAt(float u, float v, const vec3 &point) const override;
private:
	Image image;
};

