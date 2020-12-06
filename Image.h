#pragma once

struct Image
{
	unsigned char *data = nullptr;
	int x = 0, y = 0;

	unsigned char *atUV(float u, float v) const
	{
		int texelX = int(u * x);
		int texelY = int(v * y);
		return (*this).atTexel(texelX, texelY);
	}

	unsigned char *atTexel(int texelX, int texelY) const
	{
		texelX %= x;
		texelY %= y;
		return &data[3 * texelX + 3 * x * texelY];
	}

	Image(const char *path);
	Image() = default;
	~Image();
};

