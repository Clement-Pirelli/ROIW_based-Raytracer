#include "BMPWriter.h"
#include <iostream>
#include <string>
#include <algorithm>
#include "ray.h"
#include "Sphere.h"
#include "HitableList.h"
#include "randUtils.h"
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

#pragma warning(disable : 6385)

constexpr int maxBounces = 5;


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
	PathTracer()
	{
		threadAmount = std::thread::hardware_concurrency() - 1;
		//user input
		Logger::LogMessageFormatted("Welcome to the raytracer! You have %u available threads.\n", std::thread::hardware_concurrency());
		Logger::LogMessage("Please input the parameters :");
		Logger::LogMessage("x :");
		std::cin >> config.xDim;
		Logger::LogMessage("y: ");
		std::cin >> config.yDim;
		Logger::LogMessage("Samples per pixel: ");
		std::cin >> config.samples;
		Logger::LogMessage("Number of tiles per image dimension: ");
		std::cin >> config.dimTileAmount;
		config.totalTileAmount = config.dimTileAmount * config.dimTileAmount;
		Logger::LogMessage("Do you wish to display the image in a window? (y/n) ");
		char displayInWindowChar = '\0';
		std::cin >> displayInWindowChar;
		config.renderingToScreen = (displayInWindowChar == 'y');

		threads = new std::thread[threadAmount];
		config.tileWidth = config.xDim / config.dimTileAmount;
		config.tileHeight = config.yDim / config.dimTileAmount;
		config.tileCounter = 0;

		image = new color[(size_t)config.xDim * config.yDim];

		world = teapot(config);

		Logger::LogMessage("Path tracer successfully initialized!");

		if (config.renderingToScreen) screen = new RenderToScreen(image, config.xDim, config.yDim);
	}

	~PathTracer()
	{
		if (config.renderingToScreen) delete screen;
		delete world;
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
			std::thread *t = new (threads + i) std::thread(&PathTracer::trace, this, &config);
		}

		//raytracing on this core as well
		trace(&config);

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

	static Hitable *randomScene(PathTracerConfig &config) {

		constexpr int randomSpheresN = 10;

		Hitable **list = new Hitable * [randomSpheresN * 2 * randomSpheresN * 2 + 4];
		list[0] = new Sphere(vec3(0, -500.0f, 0), 500.0f, new Lambertian(new FlatTexture(vec3(0.5f, 0.5f, 0.5f))));
		int i = 1;
		for (int a = -randomSpheresN; a < randomSpheresN; a++) {
			for (int b = -randomSpheresN; b < randomSpheresN; b++) {
				float choose_mat = drand48();
				vec3 center;

				do
				{
					center = vec3(a + 0.9f * drand48(), 0.2f, b + 0.9f * drand48());
				} while (!(center - vec3(4.0f, 0.2f, 0.0f)).length() > 0.9f);

				float radius = .2f;

				Material *mat = nullptr;

				if (choose_mat < 0.6f) {
					mat = new Lambertian(new FlatTexture(randVec3()));
				}
				else if (choose_mat < 0.7f)
				{
					mat = new DiffuseLight(new FlatTexture(randVec3() * 3.0f));
				}
				else if (choose_mat < 0.8f)
				{
					list[i++] = new ConstantMedium(new Sphere(center, 1.0f, nullptr), .5f, new FlatTexture(vec3(1.0f, 1.0f, 1.0f)));
					continue;
				}
				else if (choose_mat < 0.95f) {
					mat = new Metal(vec3(0.5f * (1.0f + drand48()),
						0.5f * (1.0f + drand48()),
						0.5f * (1.0f + drand48())),
						0.5f * drand48());
				}
				else {
					mat = new Dielectric(1.5f);
				}

				list[i++] = new Sphere(
					center, radius, mat
				);
			}
		}

		list[i++] = new Sphere(vec3(0.0f, 1.0f, 0.0f), 1.0f, new Dielectric(1.5f));
		list[i++] = new Sphere(vec3(-5.0f, 1.0f, 0.0f), 2.0f, new DiffuseLight(new FlatTexture(vec3(0.4f, 0.2f, 0.1f) * 20.0f)));
		list[i++] = new Sphere(vec3(4.0f, 1.0f, 0.0f), 1.0f, new Metal(vec3(0.7f, 0.6f, 0.5f), 0.0f));

		config.lookAt = vec3(.0f, 2.0f, .0f);
		config.lookFrom = vec3(3.0f, 3.0f, -10.0f);
		config.aperture = 1.0f;
		config.distanceToFocus = 10.0f;
		config.camera = Camera(config.lookFrom, config.lookAt, vec3(.0f, 1.0f, .0f), 90.0f, config.xDim / float(config.yDim), config.aperture, config.distanceToFocus);

		return new BvhNode(list, i);
	}

	static Hitable *cornellBox(PathTracerConfig &config)
	{
		Hitable **list = new Hitable*[7];
		Material *green = new Lambertian(new FlatTexture(vec3(.0f,1.0f,.0f)));
		Material *white = new Lambertian(new FlatTexture(vec3(1.0f, 1.0f, 1.0f)));
		Material *red = new Lambertian(new FlatTexture(vec3(1.0f, .0f, .0f)));
		
		list[0] = new FlipNormals(new RectangleYZ(.0f,555.0f,.0f,555.0f,555.0f, green));
		list[1] = new RectangleYZ(.0f, 555.0f, .0f, 555.0f, .0f, red);
		list[3] = new FlipNormals(new RectangleXY(.0f, 555.0f, .0f, 555.0f, 555.0f, white));
		list[2] = new FlipNormals(new RectangleXZ(.0f, 555.0f, .0f, 555.0f, 555.0f, white));
		list[4] = new RectangleXZ(213.0f,343.0f,227.0f,332.0f,554.0f, new DiffuseLight(new FlatTexture(vec3(15.0f,15.0f,15.0f))));
		list[5] = new Sphere(vec3(213.0f, 100.0f, 300.0f), 100.0f, new Metal(vec3(1.0f,.9f,.8f), .1f));
		list[6] = new RectangleXZ(.0f, 555.0f, .0f, 555.0f, .0f, white);

		config.lookAt = vec3(278.0f, 278.0f, 0.0f);
		config.lookFrom = vec3(278.0f, 278.0f, -800.0f);
		config.aperture = .0f;
		config.distanceToFocus = 10.0f;
		config.camera = Camera(config.lookFrom, config.lookAt, vec3(.0f, 1.0f, .0f), 40.0f, config.xDim / float(config.yDim), config.aperture, config.distanceToFocus);
		
		return new BvhNode(list, 7);
	}

	static Hitable *teapot(PathTracerConfig &config)
	{
		config.lookAt = vec3(20.0f, 20.0f, .0f);
		config.lookFrom = vec3(20.0f, 100.0f, -100.0f);
		config.aperture = 1.0f;
		config.distanceToFocus = 100.0f;
		config.camera = Camera(config.lookFrom, config.lookAt, vec3(.0f, 1.0f, .0f), 90.0f, config.xDim / float(config.yDim), config.aperture, config.distanceToFocus);

		return ModelLoader::loadModel("_assets/teapot.obj", new Dielectric(1.5f));
	}

	void updateScreen(PathTracerConfig *config)
	{
		if (config->renderingToScreen)
		{
			//todo : lock and unlock
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

	static vec3 sceneColor(const ray &currentRay, Hitable *world, int depth = 0)
	{
		hitRecord record;

		if (world->hit(currentRay, .0001f, 10000.0f, record))
		{
			vec3 attenuation;
			ray scatteredRay;
			vec3 emitted = record.material->emitted(record.u, record.v, record.point);

			if (depth < maxBounces && record.material->scatter(currentRay, record, attenuation, scatteredRay))
			{
				return emitted + attenuation * sceneColor(scatteredRay, world, depth + 1);
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

	void trace(PathTracerConfig *config)
	{
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
						for (uint32_t s = 0; s < config->samples; s++)
						{
							float u = float(x + drand48()) / float(config->xDim),
								v = float(y + drand48()) / float(config->yDim);
							ray currentRay = config->camera.getRay(u, v);
							currentRay.direction.normalize();
							col += sceneColor(currentRay, world);
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
				Logger::LogMessageFormatted("Done with tile :  %u/%u\n", unsigned int(currentTile + 1), unsigned int(config->totalTileAmount));
			}
			else
			{
				return;
			}
		}
	}

	PathTracerConfig config;

	size_t threadAmount = 0;
	std::thread *threads = nullptr;



	//resources
	color *image = nullptr;
	Hitable *world = nullptr;

	//screen
	RenderToScreen *screen = nullptr;
};

int main()
{
	PathTracer pathTracer;

	pathTracer.run();

	return 0;
}