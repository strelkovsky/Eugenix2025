#pragma once

#include "Render/Model.h"

namespace Eugenix::Assets
{
	// TODO : ??
	using TVertex = Render::Vertex::PosNormalUV;

	class ObjModelLoader
	{
	public:
		Render::Model Load(const std::filesystem::path& modelPath, const std::filesystem::path& materialDir = {}, bool flipV = true)
		{
			Eugenix::Render::Model model;

			tinyobj::ObjReaderConfig readerConfig;
			readerConfig.mtl_search_path = materialDir.string();
			readerConfig.triangulate = true;
			readerConfig.vertex_color = false;

			tinyobj::ObjReader reader;
			if (!reader.ParseFromFile(modelPath.string(), readerConfig))
			{
				if (!reader.Error().empty())
					Eugenix::LogError("TinyObjReader error: {}", reader.Error());
			}
			if (!reader.Warning().empty())
				Eugenix::LogWarn("TinyObjReader warning: {}", reader.Warning());

			const auto& attrib = reader.GetAttrib();
			const auto& shapes = reader.GetShapes();
			const auto& materials = reader.GetMaterials();

			const bool isFindMaterials = !materials.empty();

			if (isFindMaterials)
			{
				Eugenix::Assets::ImageLoader imageLoader{};

				for (const auto& m : materials)
				{
					Eugenix::Render::Material out{};

					std::filesystem::path base =
						materialDir.empty() ? modelPath.parent_path() : materialDir;
					const std::filesystem::path texPath = base / m.diffuse_texname;
					const auto image = imageLoader.Load(texPath.string());

					out.diffuseTex = std::make_shared<Eugenix::Render::OpenGL::Texture2D>();
					out.diffuseTex->Create();
					out.diffuseTex->Storage(image);
					out.diffuseTex->Update(image);

					model.AddMaterial(out);
				}
			}

			struct Key
			{
				int v, vt, vn;
				bool operator==(const Key& o) const noexcept
				{
					return v == o.v && vt == o.vt && vn == o.vn;
				}
			};
			struct KeyHash
			{
				size_t operator()(const Key& k) const noexcept
				{
					size_t h = std::hash<int>{}(k.v);
					h ^= (std::hash<int>{}(k.vt) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2));
					h ^= (std::hash<int>{}(k.vn) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2));
					return h;
				}
			};

			for (const auto& shape : shapes)
			{
				struct Bucket
				{
					std::vector<TVertex> verts;
					std::vector<uint32_t> idx;
					std::unordered_map<Key, uint32_t, KeyHash> remap;
				};
				std::unordered_map<int, Bucket> buckets;

				size_t indexOffset = 0;
				const auto& fvList = shape.mesh.num_face_vertices;
				const auto& matIds = shape.mesh.material_ids;

				for (size_t f = 0; f < fvList.size(); ++f)
				{
					int fv = fvList[f];
					int matId = (f < matIds.size()) ? matIds[f] : -1;

					auto& B = buckets[matId];

					for (int v = 0; v < fv; ++v)
					{
						const tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];
						const Key key{ idx.vertex_index, idx.texcoord_index, idx.normal_index };

						auto it = B.remap.find(key);
						uint32_t dstIndex;
						if (it == B.remap.end())
						{
							TVertex vert{};

							if (idx.vertex_index >= 0)
							{
								const int vi = idx.vertex_index * 3;
								vert.pos = glm::vec3(
									attrib.vertices[vi + 0],
									attrib.vertices[vi + 1],
									attrib.vertices[vi + 2]);
							}

							if (idx.normal_index >= 0)
							{
								const int ni = idx.normal_index * 3;
								vert.normal = glm::vec3(
									attrib.normals[ni + 0],
									attrib.normals[ni + 1],
									attrib.normals[ni + 2]);
							}
							else
							{
								vert.normal = glm::vec3(0.0f, 0.0f, 1.0f);
							}

							if (idx.texcoord_index >= 0)
							{
								const int ti = idx.texcoord_index * 2;
								float u = attrib.texcoords[ti + 0];
								float v_ = attrib.texcoords[ti + 1];
								if (flipV) v_ = 1.0f - v_;
								vert.uv = glm::vec2(u, v_);
							}
							else
							{
								vert.uv = glm::vec2(0.0f);
							}

							dstIndex = static_cast<uint32_t>(B.verts.size());
							B.verts.push_back(vert);
							B.remap.emplace(key, dstIndex);
						}
						else
						{
							dstIndex = it->second;
						}

						B.idx.push_back(dstIndex);
					}

					indexOffset += fv;
				}

				for (auto& [matId, B] : buckets)
				{
					if (B.idx.empty()) continue;

					Eugenix::Render::ModelPart part;
					part.materialIndex = matId;

					std::span<const TVertex> vspan{ B.verts.data(), B.verts.size() };
					std::span<const uint32_t> ispan{ B.idx.data(), B.idx.size() };
					part.mesh.Build(vspan, ispan);

					model.AddPart(part);
				}
			}

			return model;
		}
	};
} // Eugenix::Assets
