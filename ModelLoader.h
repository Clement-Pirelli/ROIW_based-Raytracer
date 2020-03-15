#pragma once
#include "Triangle.h"
#include <vector>
#include <unordered_map>
#include <string>


class ModelLoader
{
public:

	static Hitable *loadModel(const char *filePath, Material *material);

private:

	static Hitable *makeHitable(std::vector<Triangle> &model);

	static std::unordered_map<std::string, std::vector<Triangle>> models;
	ModelLoader() = delete;
};

