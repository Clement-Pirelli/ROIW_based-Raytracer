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
	Scene(const PathTracerConfig &config);

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
	size_t xDim{};
	size_t yDim{};
	size_t samples{};
	size_t dimTileAmount{};
	size_t tileWidth{};
	size_t tileHeight{};
	size_t totalTileAmount{};

	//camera
	vec3 lookFrom{};
	vec3 lookAt{};
	float distanceToFocus{};
	float aperture{};
	Camera camera{};

	std::string modelPath{};
	std::string albedoPath{};
	std::string emissivePath{};
	std::string normalPath{};
	std::string roughnessPath{};

	bool renderingToScreen : 1 {};
	bool writeToFile : 1 {};
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

