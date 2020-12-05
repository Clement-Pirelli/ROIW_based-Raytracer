#include "PathTracer.h"
#include "BMPWriter.h"
#include <iostream>
#include <string>
#include <algorithm>
#include "ray.h"
#include "randUtils.h"
#include "Randomizer.h"
#include "Lambertian.h"
#include <atomic>
#include "Logger.h"
#include "FlatTexture.h"
#include "RenderToWindow.h"
#include "ModelLoader.h"
#include "ImageTexture.h"
#include "ImageLoader.h"

#pragma warning(disable : 6385)
namespace
{
	constexpr int maxBounces = 10;

	std::string stringFromDate()
	{
		const time_t date = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		char buffer[26];
		ctime_s(buffer, sizeof(buffer), &date);
		std::string timeString = std::string(buffer);
		std::replace(timeString.begin(), timeString.end(), ' ', '_');
		std::replace(timeString.begin(), timeString.end(), ':', '_');
		std::replace(timeString.begin(), timeString.end(), '\n', '_');
		return timeString;
	}

	float saturate(float a)
	{
		return (a < .0f) ? .0f : (a > 1.0f) ? 1.0f : a;
	}

	vec3 ACESTonemap(const vec3 &inColor)
	{
		constexpr float a = 2.51f;
		constexpr float b = 0.03f;
		constexpr float c = 2.43f;
		constexpr float d = 0.59f;
		constexpr float e = 0.14f;
		vec3 col = (inColor * (vec3(b, b, b) + inColor * a)) / (inColor * (inColor * c + vec3(d, d, d)) + vec3(e, e, e));
		return vec3(saturate(col.x()), saturate(col.y()), saturate(col.z()));
	}

	vec3 sky(const ray &currentRay)
	{
		float t = .5f * (currentRay.direction.y() + 1.0f);
		vec3 white = vec3(1.0f, 1.0f, 1.0f);
		vec3 blue = vec3(.5f, .7f, 1.0f);
		return vec3::lerp(white, blue, t);
	}

	vec3 sceneColor(const std::vector<Triangle> &triangles, const BvhNode::BvhVector &bvh, const ray &currentRay, float r1, float r2, int depth = 0)
	{
		hitRecord record;

		if (bvh[0].hit(triangles, bvh, currentRay, .0001f, 10000.0f, record))
		{
			vec3 attenuation;
			ray scatteredRay;
			vec3 emitted = record.material->emitted(record.u, record.v, record.point);

			if (depth < maxBounces && record.material->scatter(currentRay, record, attenuation, scatteredRay, r1, r2))
			{
				return emitted + attenuation * sceneColor(triangles, bvh, scatteredRay, r1, r2, depth + 1);
			}
			else
			{
				return emitted;
			}
		}
		else
		{
			return sky(currentRay);
		}
	}

	std::vector<Triangle> &triangleScene(PathTracerConfig &config)
	{
		config.lookAt = vec3(.0f, 8.0f, .0f);
		config.lookFrom = vec3(-5.0f, 12.0f, -7.0f).rotatedY(6.283272f * -.5f);
		config.aperture = .0f;
		config.distanceToFocus = .4f;
		config.camera = Camera(config.lookFrom, config.lookAt, vec3(.0f, 1.0f, .0f), 90.0f, config.xDim / float(config.yDim), config.aperture, config.distanceToFocus);

		return ModelLoader::loadModel("_assets/robot.obj", new Lambertian(new ImageTexture("_assets/robot.jpg")));
	}
}

PathTracer::PathTracer(PathTracerConfig &&givenConfig) : config(std::move(givenConfig))
{
	threads.resize(config.threadAmount);

	image = new color[(size_t)config.xDim * config.yDim];

	triangles = triangleScene(config);
	const size_t triangleNumber = triangles.size();
	Logger::LogMessageFormatted("Model successfully loaded! Model has %u triangles!", triangleNumber);

	std::vector<int> triangleIndices = std::vector<int>(triangleNumber);
	for (int i = 0; i < triangleNumber; i++) triangleIndices[i] = i;

	bvh.resize(2);

	bvh[0] = BvhNode(triangles, triangleIndices, bvh);

	Randomizer::createRandom(int(config.samples));

	Logger::LogMessage("Path tracer successfully initialized!");

	if (config.renderingToScreen) screen = new RenderToWindow(image, config.xDim, config.yDim);
}

PathTracer::~PathTracer()
{
	if (config.renderingToScreen) delete screen;
	delete[] image;
}

void PathTracer::run()
{
	Logger::LogMessage("Starting Raytracing...");
	auto timeStart = std::chrono::high_resolution_clock::now();

	//thread creation
	size_t widthPerThread = config.xDim;
	size_t heightPerThread = config.yDim;
	for (std::thread &thread : threads)
	{
		thread = std::thread(&PathTracer::trace, this, std::ref(config), std::ref(triangles), std::ref(bvh));
	}

	//raytracing on this core as well
	trace(config, triangles, bvh);

	//join threads once this core is done
	for (std::thread &thread : threads)
	{
		thread.join();
	}

	//take time
	auto timeEnd = std::chrono::high_resolution_clock::now();
	auto delta = timeEnd - timeStart;
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();

	Logger::LogMessageFormatted("Raytracing done in %u milliseconds (%u seconds)\nThank you for your time!", time, time / 1000);

	const std::string fileName = std::string("output/RaytracerOutput") + stringFromDate() + std::string(".bmp");
	//output final image to .bmp
	writeBMP(fileName.c_str(), uint32_t(config.xDim), uint32_t(config.yDim), image);

	//wait for the user to dismiss the window
	if (config.renderingToScreen) screen->handleMessagesBlocking();
}

void PathTracer::updateScreen(const PathTracerConfig &config)
{
	if (config.renderingToScreen)
	{
		screen->updateImage(image);
	}
}

void PathTracer::trace(const PathTracerConfig &config, std::vector<Triangle> &triangles, const BvhNode::BvhVector &bvh)
{
	Image blueNoise = ImageLoader::loadImage("_assets/bluenoise.png");
	float xDimF = float(config.xDim);
	float yDimF = float(config.yDim);

	while (true)
	{
		//get the current tile, add 1 to the counter
		uint32_t currentTile = tileCounter.fetch_add(1);
		//if there is still work to be done
		if (currentTile < config.totalTileAmount)
		{
			//tile dimension calculations
			size_t yStart = (currentTile / config.dimTileAmount) * config.tileHeight;
			size_t xStart = (currentTile % config.dimTileAmount) * config.tileWidth;

			//for every pixel in the tile
			for (size_t y = yStart; y < yStart + config.tileHeight; y++)
			{
				for (size_t x = xStart; x < xStart + config.tileWidth; x++)
				{
					//trace samples, average
					vec3 col = vec3(.0f, .0f, .0f);

					//blue noise tiling
					unsigned char *imageVal = blueNoise.atTexel(static_cast<int>(x), static_cast<int>(y));
					float noise1 = float(imageVal[0]) / 255.0f;
					float noise2 = float(imageVal[1]) / 255.0f;

					for (uint32_t s = 0; s < config.samples; s++)
					{
						const float u = (float(x) + drand48()) / xDimF;
						const float v = (float(y) + drand48()) / yDimF;

						ray currentRay = config.camera.getRay(u, v);
						currentRay.direction.normalize();
						float _;
						vec3 sampleRandom = Randomizer::getRandom(s);
						col += sceneColor(triangles, bvh, currentRay, modf(sampleRandom.x() + noise1, &_), modf(sampleRandom.y() + noise2, &_));
					}
					col /= float(config.samples);

					//tonemapping
					col = ACESTonemap(col);

					//gamma correction
					col = vec3(
						powf(col.x(), 1.0f / 2.2f),
						powf(col.y(), 1.0f / 2.2f),
						powf(col.z(), 1.0f / 2.2f));


					color currentCol
					{
						.b = uint8_t(255.99f * col.b()),
						.g = uint8_t(255.99f * col.g()),
						.r = uint8_t(255.99f * col.r()),
						.a = 255
					};
					image[y * config.xDim + x] = currentCol;
				}
			}
			updateScreen(config);
			Logger::LogMessageFormatted("Done with tile :  %u/%u", unsigned int(currentTile + 1), unsigned int(config.totalTileAmount));
		}
		else
		{
			ImageLoader::freeImage(blueNoise);
			return;
		}
	}
}
