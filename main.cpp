#include <thread>
#include "PathTracer.h"
#include "Logger.h"
#include <iostream>

#define INPUT_CHECK(conditions, errorMessage) if(!(conditions)){ Logger::LogError(errorMessage); continue;};

int main()
{
	PathTracerConfig config;
	config.threadAmount = std::thread::hardware_concurrency() - 1;

	//user input
	Logger::LogMessageFormatted("Welcome to the raytracer! You have %u available threads.\n", std::thread::hardware_concurrency());
	while (true)
	{

		Logger::LogMessage("Please input the parameters :");
		Logger::LogMessage("x :");
		std::cin >> config.xDim;

		INPUT_CHECK(config.xDim > 0, "Invalid x value! Please enter a value greater than 0");

		Logger::LogMessage("y: ");
		std::cin >> config.yDim;

		INPUT_CHECK(config.yDim > 0, "Invalid y value! Please enter a value greater than 0");

		Logger::LogMessage("Samples per pixel: ");
		std::cin >> config.samples;

		INPUT_CHECK(config.samples > 0, "Invalid samples value! Please enter a value greater than 0");

		Logger::LogMessage("Number of tiles per image dimension: ");
		std::cin >> config.dimTileAmount;

		INPUT_CHECK(config.xDim >= config.dimTileAmount && config.yDim >= config.dimTileAmount, "Invalid tile value! Please enter a value smaller than or equal to the amount of pixels!");

		config.totalTileAmount = config.dimTileAmount * config.dimTileAmount;
		Logger::LogMessage("Do you wish to display the image in a window? (y/n) ");
		char displayInWindowChar = '\0';
		std::cin >> displayInWindowChar;
		config.renderingToScreen = (displayInWindowChar == 'y');

		break;
	}

	config.tileWidth = config.xDim / config.dimTileAmount;
	config.tileHeight = config.yDim / config.dimTileAmount;
	PathTracer pathTracer(std::move(config));

	pathTracer.run();

	return 0;
}