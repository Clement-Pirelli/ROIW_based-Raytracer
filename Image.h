#pragma once
#include "color.h"

struct Image
{
	unsigned char *data = nullptr;
	int x = 0, y = 0;

	color atUV(float u, float v) const;

	color atTexel(int texelX, int texelY) const;

	Image(const char *path);
	Image() = default;
	~Image();
};

