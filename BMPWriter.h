#ifndef BMP_WRITER_H_DEFINED
#define BMP_WRITER_H_DEFINED
#include "color.h"
void writeBMP(const char *path, uint32_t xPixelCount, uint32_t yPixelCount, color *contents);

#endif