#include "ImageLoader.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Image ImageLoader::loadImage(const char *path)
{
	Image returnImage;
	int nn;
	returnImage.data = stbi_load(path, &returnImage.x, &returnImage.y, &nn, STBI_rgb);
	return returnImage;
}

void ImageLoader::freeImage(Image &image)
{
	stbi_image_free(image.data);
	image.x = 0;
	image.y = 0;
}
