#ifndef BMP_WRITER_H_DEFINED
#define BMP_WRITER_H_DEFINED
#include <cstdint>

union color
{
	struct
	{
		uint8_t b, g, r, a;
	};
	uint8_t data[4];
};



void writeBMP(const char *path, uint32_t xPixelCount, uint32_t yPixelCount, color *contents);

#endif