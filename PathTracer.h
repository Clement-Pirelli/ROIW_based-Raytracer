#pragma once
#include <vector>
#include "BvhNode.h"
#include "Camera.h"
#include <thread>
#include "Triangle.h"

struct color;
class RenderToWindow;

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
	size_t threadAmount = 0;

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
	PathTracer(PathTracerConfig &&givenConfig);

	~PathTracer();

	void run();

private:

	void updateScreen(const PathTracerConfig &config);

	void trace(const PathTracerConfig &config, std::vector<Triangle> &triangles, const BvhNode::BvhVector &bvh);


	//resources
	BvhNode::BvhVector bvh;
	std::vector<Triangle> triangles;
	color *image = nullptr;
	std::atomic<uint32_t> tileCounter = 0;


	//config
	PathTracerConfig config;

	size_t threadAmount = 0;
	std::vector<std::thread> threads;

	//screen
	RenderToWindow *screen = nullptr;
};
