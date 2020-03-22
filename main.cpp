#include "BMPWriter.h"
#include <iostream>
#include <string>
#include <algorithm>
#include "ray.h"
#include "randUtils.h"
#include "Randomizer.h"
#include "AllMaterials.h"
#include "Camera.h"
#include <thread>
#include <atomic>
#include "Logger.h"
#include "BvhNode.h"
#include "FlatTexture.h"
#include "RenderToScreen.h"
#include "ConstantMedium.h"
#include "Rectangle.h"
#include "FlipNormals.h"
#include "ModelLoader.h"
#include "ImageTexture.h"
#include "ImageLoader.h"
#include "AlignedAllocator.h"


#define INPUT_CHECK(conditions, errorMessage) if(!(conditions)){ Logger::LogError(errorMessage); continue;};


#pragma warning(disable : 6385)

constexpr int maxBounces = 10;
std::vector<Triangle> _; //needed so that the path tracer may hold a ref

struct PathTracerConfig
{
	//image variables
	size_t xDim = 0;
	size_t yDim = 0;
	size_t samples = 0;
	size_t tileWidth = 0;
	size_t tileHeight = 0;
	size_t dimTileAmount = 0;
	size_t totalTileAmount = 0;
	std::atomic<uint32_t> tileCounter = 0;

	//camera
	vec3 lookFrom = vec3();
	vec3 lookAt = vec3();
	float distanceToFocus = .0f;
	float aperture = .0f;
	Camera camera = Camera();

	bool renderingToScreen = false;
};

class PathTracer
{
public:
	PathTracer() : triangles(_)
	{
		threadAmount = std::thread::hardware_concurrency() - 1;

		//user input
		Logger::LogMessageFormatted("Welcome to the raytracer! You have %u available threads.\n", std::thread::hardware_concurrency());
		while(true)
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

		threads = new std::thread[threadAmount];
		config.tileWidth = config.xDim / config.dimTileAmount;
		config.tileHeight = config.yDim / config.dimTileAmount;
		config.tileCounter = 0;

		image = new color[(size_t)config.xDim * config.yDim];

		triangles = triangleScene(config);
		size_t triangleNumber = triangles.size();
		Logger::LogMessageFormatted("Model successfully loaded! Model has %u triangles!", triangleNumber);
		std::vector<int> triangleIndices = std::vector<int>(triangleNumber);
		for (int i = 0; i < triangleNumber; i++) triangleIndices[i] = i;

		bvh.resize(2);

		bvh[0] = BvhNode(triangles, triangleIndices, bvh);

		Randomizer::createRandom(int(config.samples));

		Logger::LogMessage("Path tracer successfully initialized!");

		if (config.renderingToScreen) screen = new RenderToScreen(image, config.xDim, config.yDim);
	}

	~PathTracer()
	{
		if (config.renderingToScreen) delete screen;
		delete[] image;
		delete[] threads;
	}

	void run()
	{
		Logger::LogMessage("Starting Raytracing...");
		auto timeStart = std::chrono::high_resolution_clock::now();

		//thread creation
		size_t widthPerThread = config.xDim;
		size_t heightPerThread = config.yDim;
		for (size_t i = 0; i < threadAmount; i++)
		{
			std::thread *t = new (threads + i) std::thread(&PathTracer::trace, this, &config, std::ref(triangles), std::ref(bvh));
		}

		//raytracing on this core as well
		trace(&config, triangles, bvh);

		//join threads once this core is done
		for (int i = 0; i < threadAmount; i++)
		{
			threads[i].join();
		}

		//take time
		auto timeEnd = std::chrono::high_resolution_clock::now();
		auto delta = timeEnd - timeStart;
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();

		Logger::LogMessageFormatted("Raytracing done in %u milliseconds (%u seconds)\nThank you! for your time!", time, time/1000);

		//get the current date at the right format to save it
		time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		char str[26];
		ctime_s(str, sizeof str, &t);
		std::string timeStr = std::string(str);
		std::replace(timeStr.begin(), timeStr.end(), ' ', '_');
		std::replace(timeStr.begin(), timeStr.end(), ':', '_');
		std::replace(timeStr.begin(), timeStr.end(), '\n', '_');

		//output final image to .bmp
		writeBMP((std::string("output/RaytracerOutput") + timeStr + std::string(".bmp")).c_str(), uint32_t(config.xDim), uint32_t(config.yDim), image);

		//wait for the user to dismiss the window
		if (config.renderingToScreen) screen->handleMessagesBlocking();
	}

private:


	static float saturate(float a)
	{
		return (a < .0f) ? .0f : (a > 1.0f) ? 1.0f : a;
	}

	static vec3 ACESTonemap(const vec3 &inColor)
	{
		float a = 2.51f;
		float b = 0.03f;
		float c = 2.43f;
		float d = 0.59f;
		float e = 0.14f;
		vec3 col = (inColor * (vec3(b, b, b) + inColor * a)) / (inColor * (inColor * c + vec3(d, d, d)) + vec3(e, e, e));
		return vec3(saturate(col.x), saturate(col.y), saturate(col.z));
	}

	static std::vector<Triangle> &triangleScene(PathTracerConfig &config)
	{
		config.lookAt = vec3(.0f, 8.0f, .0f);
		config.lookFrom = vec3(-5.0f, 12.0f, -7.0f).rotateY(6.283272f * -.5f);
		config.aperture = .0f;
		config.distanceToFocus = .4f;
		config.camera = Camera(config.lookFrom, config.lookAt, vec3(.0f, 1.0f, .0f), 90.0f, config.xDim / float(config.yDim), config.aperture, config.distanceToFocus);

		return ModelLoader::loadModel("_assets/robot.obj", new Lambertian(new ImageTexture("_assets/robot.jpg")));
	}

	void updateScreen(PathTracerConfig *config)
	{
		if (config->renderingToScreen)
		{
			screen->updateImage(image);
		}
	}

	static vec3 sky(const ray &currentRay)
	{
		float t = .5f * (currentRay.direction.y + 1.0f);
		vec3 white = vec3(1.0f, 1.0f, 1.0f);
		vec3 blue = vec3(.5f, .7f, 1.0f);
		return vec3::lerp(white, blue, t);
	}

	static vec3 sceneColor(std::vector<Triangle> &triangles, BvhNode::BvhVector &bvh, const ray &currentRay, float r1, float r2, int depth = 0)
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

	void trace(PathTracerConfig *config, std::vector<Triangle> &triangles, BvhNode::BvhVector bvh)
	{
		Image blueNoise = ImageLoader::loadImage("_assets/bluenoise.png");
		float xDimF = float(config->xDim);
		float yDimF = float(config->yDim);

		while (true)
		{
			//get the current tile, add 1 to the counter
			uint32_t currentTile = config->tileCounter.fetch_add(1);
			//if there is still work to be done
			if (currentTile < config->totalTileAmount)
			{
				//tile dimension calculations
				size_t yStart = (currentTile / config->dimTileAmount) * config->tileHeight;
				size_t xStart = (currentTile % config->dimTileAmount) * config->tileWidth;

				//for every pixel in the tile
				for (size_t y = yStart; y < yStart + config->tileHeight; y++)
				{
					for (size_t x = xStart; x < xStart + config->tileWidth; x++)
					{
						//trace samples, average
						vec3 col = vec3(.0f, .0f, .0f);

						//blue noise tiling
						unsigned char *imageVal = blueNoise.atTexel(static_cast<int>(x), static_cast<int>(y));
						float noise1 = float(imageVal[0]) / 255.0f;
						float noise2 = float(imageVal[1]) / 255.0f;

						for (uint32_t s = 0; s < config->samples; s++)
						{
							float u = (float(x)) / xDimF,
								v = (float(y)) / yDimF;

							ray currentRay = config->camera.getRay(u, v);
							currentRay.direction.normalize();
							float _;
							vec3 sampleRandom = Randomizer::getRandom(s);
							col += sceneColor(triangles, bvh, currentRay, modf(sampleRandom.x + noise1, &_), modf(sampleRandom.y + noise2, &_));
						}
						col /= float(config->samples);

						//tonemapping
						col = ACESTonemap(col);

						//gamma correction
						col = vec3(
							powf(col.x, 1.0f / 2.2f),
							powf(col.y, 1.0f / 2.2f),
							powf(col.z, 1.0f / 2.2f));


						color currentCol;
						currentCol.r = uint8_t(255.99f * col.r);
						currentCol.g = uint8_t(255.99f * col.g);
						currentCol.b = uint8_t(255.99f * col.b);
						currentCol.a = 255;
						image[y * config->xDim + x] = currentCol;
					}
				}
				updateScreen(config);
				Logger::LogMessageFormatted("Done with tile :  %u/%u", unsigned int(currentTile + 1), unsigned int(config->totalTileAmount));
			}
			else
			{
				ImageLoader::freeImage(blueNoise);
				return;
			}
		}
	}




	//resources
	BvhNode::BvhVector bvh;
	std::vector<Triangle> &triangles;
	color *image = nullptr;


	//config
	PathTracerConfig config;

	size_t threadAmount = 0;
	std::thread *threads = nullptr;

	//screen
	RenderToScreen *screen = nullptr;
};

int main()
{
	PathTracer pathTracer;

	pathTracer.run();

	return 0;
}