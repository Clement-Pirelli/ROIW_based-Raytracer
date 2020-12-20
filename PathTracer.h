#pragma once
#include <vector>
#include "BvhNode.h"
#include "Camera.h"
#include <thread>
#include "Triangle.h"

#include "ConstantMedium.h"
#include "AllMaterials.h"

struct PathTracerConfig;
struct Scene
{
	Scene(PathTracerConfig &config);

	bool hit(const ray &givenRay, float minT, float maxT, hitRecord &record) const
	{
		return bvh.hit(givenRay, minT, maxT, record);
	}

	size_t triangleCount() const
	{
		return bvh.triangleCount();
	}

private:

	BVH bvh;
};


struct color;
class RenderToWindow;

struct PathTracerConfig
{
	//image variables
	size_t xDim = 0;
	size_t yDim = 0;
	size_t samples = 0;
	size_t dimTileAmount = 0;
	size_t tileWidth = 0;
	size_t tileHeight = 0;
	size_t totalTileAmount = 0;

	//camera
	vec3 lookFrom = vec3();
	vec3 lookAt = vec3();
	float distanceToFocus = .0f;
	float aperture = .0f;
	Camera camera = Camera();

	bool renderingToScreen : 1 = false;
	bool writeToFile : 1 = false;
};

class PathTracer
{
public:
	PathTracer(PathTracerConfig givenConfig);


private:

	void run(PathTracerConfig config);
	void updateScreen(const PathTracerConfig &config);

	void trace(const PathTracerConfig &config, const Scene& scene);


	//resources
	Scene scene;
	color *image = nullptr;
	std::atomic<uint32_t> tileCounter = 0;

	std::vector<std::thread> threads;

	//screen
	RenderToWindow *screen = nullptr;
};

