#pragma once

#include "Render/Mesh.h"
#include "Render/Material.h"

namespace Eugenix::Render
{
	struct ModelPart final
	{
		Mesh mesh;
		int materialIndex = -1;
	};

	struct Model final
	{
		void AddPart(const Eugenix::Render::ModelPart& part)
		{
			_parts.push_back(part);
		}

		void AddPart(Eugenix::Render::ModelPart&& part)
		{
			_parts.push_back(std::move(part));
		}

		void AddPart(const Eugenix::Render::Mesh& mesh, int materialIndex = -1)
		{
			_parts.push_back(Eugenix::Render::ModelPart{ mesh, materialIndex });
		}

		void AddPart(Eugenix::Render::Mesh&& mesh, int materialIndex = -1)
		{
			_parts.push_back(Eugenix::Render::ModelPart{ std::move(mesh), materialIndex });
		}

		void AddMaterial(const Eugenix::Render::Material& material)
		{
			_materials.push_back(material);
		}

		void Render()
		{
			for (auto& part : _parts)
			{
				if (part.materialIndex >= 0 && part.materialIndex < (int)_materials.size())
				{
					auto& diffuse = _materials[part.materialIndex].diffuseTex;
					auto& specular = _materials[part.materialIndex].specularTex;

					if (diffuse) 
						diffuse->Bind(0); // TODO : use TextureLocation
					if (specular) 
						specular->Bind(1); // TODO : use TextureLocation
				}

				part.mesh.Bind();
				part.mesh.Draw();
			}
		}

		void Destroy()
		{
			for (auto& p : _parts)
			{
				p.mesh.Destroy();
			}

			for (auto& m : _materials)
			{
				m.Destroy();
			}

			_parts.clear();
			_materials.clear();
		}

	private:
		std::vector<Eugenix::Render::ModelPart> _parts;
		std::vector<Eugenix::Render::Material> _materials;
	};

	inline Eugenix::Render::Model CreateModelFromMeshes(std::vector<Eugenix::Render::Mesh>&& meshes)
	{
		Eugenix::Render::Model model;

		for (auto& mesh : meshes)
		{
			model.AddPart(Eugenix::Render::ModelPart{ std::move(mesh), -1 });
		}

		return model;
	}

} // namespace Eugenix::Render
