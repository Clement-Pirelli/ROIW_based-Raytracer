#include "PathTracer.h"
#include "BMPWriter.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <numeric>
#include "ray.h"
#include "randUtils.h"
#include "Randomizer.h"
#include <atomic>
#include "Logger.h"
#include "FlatTexture.h"
#include "RenderToWindow.h"
#include "ModelLoader.h"
#include "ImageTexture.h"
#include "Time.h"
#include "BlueNoise.h"

#include "PBR.h"

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
		const float t = .5f * (currentRay.direction.y() + 1.0f);
		const vec3 grey = vec3(1.0f, 1.0f, 1.0f) * .1f;
		const vec3 blue = vec3(.5f, .7f, 1.0f) * .1f;
		return vec3::lerp(grey, blue, t);
	}

	vec3 sceneColor(const Scene& scene, const ray &currentRay, float r1, float r2, int depth = 0)
	{
		hitRecord record;

		if (scene.hit(currentRay, .0001f, 10000.0f, record))
		{
			vec3 attenuation;
			ray scatteredRay;
			vec3 emitted = record.material->emitted(record.u, record.v, record.point);

			if (depth < maxBounces && record.material->scatter(currentRay, record, attenuation, scatteredRay, r1, r2))
			{
				return emitted + attenuation * sceneColor(scene, scatteredRay, r1, r2, depth + 1);
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
}

PathTracer::PathTracer(PathTracerConfig config) : scene(config)
{
	threads.resize(std::thread::hardware_concurrency() - 1); // -1 because the main thread is also counted

	image = new color[(size_t)config.xDim * config.yDim];

	const size_t triangleNumber = scene.triangleCount();
	Logger::LogMessageFormatted("Model successfully loaded! Model has %u triangles!", triangleNumber);

	Randomizer::createRandom(int(config.samples));

	Logger::LogMessage("Path tracer successfully initialized!");

	if (config.renderingToScreen) screen = new RenderToWindow(image, config.xDim, config.yDim);

	run(config);

	delete screen;
	delete[] image;
}

void PathTracer::run(PathTracerConfig config)
{
	Logger::LogMessage("Starting Raytracing...");
	Time timeStart = Time::now();

	//thread creation
	size_t widthPerThread = config.xDim;
	size_t heightPerThread = config.yDim;
	for (std::thread &thread : threads)
	{
		thread = std::thread(&PathTracer::trace, this, std::ref(config), std::ref(scene));
	}

	//raytracing on this core as well
	trace(config, scene);

	//join threads once this core is done
	for (std::thread &thread : threads)
	{
		thread.join();
	}

	Time timeEnd = Time::now();
	Time delta = timeEnd - timeStart;

	Logger::LogMessageFormatted("Raytracing done in %u milliseconds (%f seconds)\nThank you for your time!", delta.asMilliseconds().amount, delta.asSeconds());

	const std::string fileName = std::string("output/RaytracerOutput") + stringFromDate() + std::string(".bmp");
	//output final image to .bmp
	if(config.writeToFile) writeBMP(fileName.c_str(), uint32_t(config.xDim), uint32_t(config.yDim), image);

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

void PathTracer::trace(const PathTracerConfig &config, const Scene& scene)
{
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
					color imageVal = blueNoise.atTexel(static_cast<int>(x), static_cast<int>(y));
					float noise1 = float(imageVal.r) / 255.0f;
					float noise2 = float(imageVal.g) / 255.0f;

					for (uint32_t s = 0; s < config.samples; s++)
					{
						const float u = (float(x) + drand48()) / xDimF;
						const float v = (float(y) + drand48()) / yDimF;

						ray currentRay = config.camera.getRay(u, v);
						currentRay.direction.normalize();
						vec3 sampleRandom = Randomizer::getRandom(s);
						float _;
						col += sceneColor(scene, currentRay, modf(sampleRandom.x() + noise1, &_), modf(sampleRandom.y() + noise2, &_));
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
			return;
		}
	}
}

Scene::Scene(const PathTracerConfig &config) 
	: bvh(ModelLoader::loadModel(config.modelPath.c_str())
	, new PBRMaterial(
		new ImageTexture(config.albedoPath.c_str()), 
		new ImageTexture(config.emissivePath.c_str()), 
		new ImageTexture(config.normalPath.c_str()), 
		new ImageTexture(config.roughnessPath.c_str())
	))
{

}

