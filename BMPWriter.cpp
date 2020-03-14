#include "BMPWriter.h"

#include <fstream>

#pragma pack(push, 1)

//from : https://solarianprogrammer.com/2018/11/19/cpp-reading-writing-bmp-images/
struct BMPFileHeader {
	uint16_t file_type{ 0x4D42 };          // File type always BM which is 0x4D42
	uint32_t file_size{ 0 };               // Size of the file (in bytes)
	uint16_t reserved1{ 0 };               // Reserved, always 0
	uint16_t reserved2{ 0 };               // Reserved, always 0
	uint32_t offset_data{ 0 };             // Start position of pixel data (bytes from the beginning of the file)
};

struct BMPInfoHeader {
	uint32_t size{ 0 };                      // Size of this header (in bytes)
	int32_t width{ 0 };                      // width of bitmap in pixels
	int32_t height{ 0 };                     // width of bitmap in pixels
											 //       (if positive, bottom-up, with origin in lower left corner)
											 //       (if negative, top-down, with origin in upper left corner)
	uint16_t planes{ 1 };                    // No. of planes for the target device, this is always 1
	uint16_t bit_count{ 0 };                 // No. of bits per pixel
	uint32_t compression{ 0 };               // 0 or 3 - uncompressed. THIS PROGRAM CONSIDERS ONLY UNCOMPRESSED BMP images
	uint32_t size_image{ 0 };                // 0 - for uncompressed images
	int32_t x_pixels_per_meter{ 0 };
	int32_t y_pixels_per_meter{ 0 };
	uint32_t colors_used{ 0 };               // No. color indexes in the color table. Use 0 for the max number of colors allowed by bit_count
	uint32_t colors_important{ 0 };          // No. of colors used for displaying the bitmap. If 0 all colors are required

};

struct BMPColorHeader {
	uint32_t red_mask{ 0x00ff0000 };         // Bit mask for the red channel
	uint32_t green_mask{ 0x0000ff00 };       // Bit mask for the green channel
	uint32_t blue_mask{ 0x000000ff };        // Bit mask for the blue channel
	uint32_t alpha_mask{ 0xff000000 };       // Bit mask for the alpha channel
	uint32_t color_space_type{ 0x73524742 }; // Default "sRGB" (0x73524742)
	uint32_t unused[16]{ 0 };                // Unused data for sRGB color space
};

struct BMPHeader
{
	BMPFileHeader fileHeader;
	BMPInfoHeader infoHeader;
private:
	BMPColorHeader colorHeader;
};

#pragma pack(pop)


void writeBMP(const char *path, uint32_t xPixelCount, uint32_t yPixelCount, color *contents)
{
	uint32_t headerSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
	uint32_t contentsSize = xPixelCount * yPixelCount * sizeof(color);

	BMPFileHeader fileHeader;

	BMPInfoHeader infoHeader;
	infoHeader.bit_count = sizeof(color) * 8;
	infoHeader.width = xPixelCount;
	infoHeader.height = yPixelCount;
	infoHeader.size = sizeof(BMPInfoHeader);

	BMPColorHeader colorHeader;

	BMPHeader header;
	header.fileHeader.file_size = headerSize + contentsSize;
	header.fileHeader.offset_data = headerSize;
	header.infoHeader.bit_count = sizeof(color) * 8;
	header.infoHeader.width = xPixelCount;
	header.infoHeader.height = yPixelCount;
	header.infoHeader.size = sizeof(BMPInfoHeader);

	std::ofstream file(path, std::ios::binary);
	if (file.fail()) throw std::runtime_error("File open failed!");
	file.clear();
	file.write(reinterpret_cast<const char *>(&header), sizeof(header));
	file.write(reinterpret_cast<const char *>(contents), contentsSize);
	file.close();
}
