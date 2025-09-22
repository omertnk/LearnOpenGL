#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "Mesh.h"
#include <string>
#include <iostream>
#include <vector>

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);


class Model
{
public:
	Model(std::string const& path, bool gamma = false) : gammaCorrection(gamma)
	{
		loadModel(path);
	}

	void Draw(Shader& shader);

	std::vector<Texture> textures_loaded;  // stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	std::vector<Mesh> meshes;
	std::string directory;
	bool gammaCorrection;

private:
	void loadModel(std::string const& path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

