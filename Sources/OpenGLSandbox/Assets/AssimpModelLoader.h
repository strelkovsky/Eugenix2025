#pragma once

#include <filesystem>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include "Engine/Core/Log.h"

namespace
{
	constexpr uint32_t ImportFlags = aiProcess_Triangulate 
		| aiProcess_FlipUVs
		| aiProcess_GenSmoothNormals
		| aiProcess_JoinIdenticalVertices;
}

namespace Eugenix::Assets
{
	class AssimpModelLoader final
	{
	public:
		const aiScene* Load(const std::filesystem::path& path)
		{
			auto scene = _importer.ReadFile(path.string(), ImportFlags);
			if (!scene)
			{
				LogError("Error parsing '%s': '%s'", path.string().c_str(), _importer.GetErrorString());
				return nullptr;
			}

			return _importer.ReadFile(path.string(), ImportFlags);
		}
	private:
		Assimp::Importer _importer;
	};
} // namespace Eugenix::Assets