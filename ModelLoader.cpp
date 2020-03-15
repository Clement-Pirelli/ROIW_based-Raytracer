#include "ModelLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "BvhNode.h"

std::unordered_map<std::string, std::vector<Triangle>> ModelLoader::models = std::unordered_map<std::string, std::vector<Triangle>>();

Hitable *ModelLoader::loadModel(const char *filePath, Material *material)
{
	std::string filePathStr = std::string(filePath);
	if (models.count(filePathStr)) 
	{
		return makeHitable(models[filePathStr]);
	}

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::vector<Vertex> vertices;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath)) {
		throw std::runtime_error(warn + err);
	}

	if (attrib.vertices.size() == 0)
	{
		throw std::runtime_error("Model at " + filePathStr + " has no vertices!");
	}

	if (attrib.texcoords.size() == 0)
	{
		throw std::runtime_error("Model at " + filePathStr + " has no UVs!");
	}

	for (const auto &shape : shapes) {
		for (const auto &index : shape.mesh.indices) {


			Vertex vertex = {};

			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.u = attrib.texcoords[2 * index.texcoord_index + 0];
			vertex.v = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];

			vertices.push_back(vertex);
		}
	}


	if (vertices.size() % 3 != 0) throw std::runtime_error("Model at " + filePathStr + " isn't triangulated properly!");
	
	std::vector<Triangle> triangles;
	for(int i = 0; i < vertices.size(); i+=3)
	{
		triangles.push_back(Triangle(vertices.data()+i, material));
	}

	models[filePathStr] = triangles;

	return makeHitable(models[filePathStr]);
}

Hitable *ModelLoader::makeHitable(std::vector<Triangle> &model)
{
	Hitable **list = new Hitable*[model.size()];
	for(size_t i = 0; i < model.size(); i++)
	{
		list[i] = &model[i];
	}
	
	return new BvhNode(list, model.size());
}
