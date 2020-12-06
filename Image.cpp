#include "Image.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


Image::Image(const char *path)
{
	int nn;
	data = stbi_load(path, &x, &y, &nn, STBI_rgb);
}

Image::~Image()
{
	stbi_image_free(data);
}
