#pragma once

struct Image
{
	unsigned char *data = nullptr;
	int x = 0, y = 0;

	unsigned char *atUV(float u, float v) const
	{
		int texelX = u * x;
		int texelY = v * y;
		return (*this).atTexel(texelX, texelY);
	}

	unsigned char *atTexel(int texelX, int texelY) const
	{
		texelX %= x;
		texelY %= y;
		return &data[3 * texelX + 3 * x * texelY];
	}
};


class ImageLoader
{
public:

	static Image loadImage(const char *path);
	static void freeImage(Image &image);

private:


	ImageLoader() = delete;
};

