#pragma once

#include <map>
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "OGLDevMath.h"

#include "Assets/ImageLoader.h"
#include "Render/OpenGL/EugenixGL.h"
#include "Render/OpenGL/Texture2D.h"

static constexpr uint32_t position_location = 0;
static constexpr uint32_t tex_coord_location = 1;
static constexpr uint32_t normal_location = 2;

static constexpr uint32_t invalid_material = std::numeric_limits<uint32_t>::max();

static constexpr uint32_t assimp_load_flags =
    aiProcess_Triangulate |
    aiProcess_GenSmoothNormals |
    aiProcess_FlipUVs |
    aiProcess_JoinIdenticalVertices;

struct MeshVertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

// CPU-mesh
struct MeshData
{
    struct SubMesh
    {
        uint32_t numIndices{};
        uint32_t baseVertex{};
        uint32_t baseIndex{};
        uint32_t materialIndex{ invalid_material };
    };

    std::vector<MeshVertex> vertices;
    std::vector<uint32_t> indicies;
    std::vector<MeshData::SubMesh> subMeshes;
};

class MeshLoader
{
public:
    MeshData Load(const std::filesystem::path& filename)
    {

    }
};

// GPU-mesh
class BasicMesh
{
public:
    BasicMesh() = default;

    BasicMesh(const BasicMesh&) = delete;
    BasicMesh& operator=(const BasicMesh&) = delete;

    ~BasicMesh()
    {
        clear();
    }

    void CreateFromData(const MeshData& data)
    {

    }

    bool LoadMesh(const std::filesystem::path& filename)
    {
        clear();

        _vao.Create();
        _vbo.Create();
        _ibo.Create();

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(filename.string(), assimp_load_flags);

        if (!scene)
        {
            printf("Error parsing '%s': '%s'\n", filename.string().c_str(), importer.GetErrorString());
            clear();
            return false;
        }

        if (!initFromScene(scene, filename))
        {
            clear();
            return false;
        }

        return true;
    }

    void Render()
    {
        _vao.Bind();

        for (const auto& submesh : _meshData.subMeshes)
        {
            uint32_t MaterialIndex = submesh.materialIndex;

            assert(MaterialIndex < _textures.size());

            if (_textures[MaterialIndex].NativeHandle() != 0) 
            {
                _textures[MaterialIndex].Bind(0);
            }

            glDrawElementsBaseVertex(GL_TRIANGLES,
                submesh.numIndices,
                GL_UNSIGNED_INT,
                (void*)(sizeof(uint32_t) * submesh.baseIndex),
                submesh.baseVertex);
        }
    }

    void Render(uint32_t NumInstances, const glm::mat4* WVPMats, const glm::mat4* WorldMats)
    {
        //glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WVP_MAT_VB]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * NumInstances, WVPMats, GL_DYNAMIC_DRAW);

        //glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WORLD_MAT_VB]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * NumInstances, WorldMats, GL_DYNAMIC_DRAW);

        //glBindVertexArray(m_VAO);

        //for (uint32_t i = 0; i < _subMeshes.size(); i++) 
        //{
        //    const uint32_t MaterialIndex = _subMeshes[i].MaterialIndex;

        //    assert(MaterialIndex < _textures.size());

        //    if (_textures[MaterialIndex].NativeHandle() > 0) 
        //    {
        //        _textures[MaterialIndex].Bind(GL_TEXTURE0);
        //    }

        //    glDrawElementsInstancedBaseVertex(GL_TRIANGLES,
        //        _subMeshes[i].NumIndices,
        //        GL_UNSIGNED_INT,
        //        (void*)(sizeof(uint32_t) * _subMeshes[i].BaseIndex),
        //        NumInstances,
        //        _subMeshes[i].BaseVertex);
        //}

        //// Make sure the VAO is not changed from the outside
        //glBindVertexArray(0);
    }

    WorldTransform& GetWorldTransform() { return _worldTransform; }

private:
    void clear()
    {
        for (auto& texture : _textures)
        {
            if (texture.NativeHandle() != 0)
            {
                texture.Destroy();
            }
        }

        if (_vbo.NativeHandle() != 0) { _vbo.Destroy(); }
        if (_ibo.NativeHandle() != 0) { _ibo.Destroy(); }
        if (_vao.NativeHandle() != 0) { _vao.Destroy(); }

        _meshData.subMeshes.clear();
        _textures.clear();

        _meshData.vertices.clear();
        _meshData.indicies.clear();
    }

    bool initFromScene(const aiScene* pScene, const std::filesystem::path& path)
    {
        _meshData.subMeshes.resize(pScene->mNumMeshes);
        _textures.resize(pScene->mNumMaterials);

        uint32_t numVertices = 0;
        uint32_t numIndices = 0;

        countVerticesAndIndices(pScene, numVertices, numIndices);
        reserveSpace(numVertices, numIndices);
        initAllMeshes(pScene);

        if (!initMaterials(pScene, path))
        {
            return false;
        }

        populateBuffers();

        return true;
    }

    void countVerticesAndIndices(const aiScene* pScene, uint32_t& NumVertices, uint32_t& NumIndices)
    {
        for (uint32_t i = 0; i < _meshData.subMeshes.size(); i++)
        {
            _meshData.subMeshes[i].materialIndex = pScene->mMeshes[i]->mMaterialIndex;
            _meshData.subMeshes[i].numIndices = pScene->mMeshes[i]->mNumFaces * 3;
            _meshData.subMeshes[i].baseVertex = NumVertices;
            _meshData.subMeshes[i].baseIndex = NumIndices;

            NumVertices += pScene->mMeshes[i]->mNumVertices;
            NumIndices += _meshData.subMeshes[i].numIndices;
        }
    }

    void reserveSpace(uint32_t numVertices, uint32_t numIndices)
    {
        _meshData.vertices.reserve(numVertices);
        _meshData.indicies.reserve(numIndices);
    }

    void initAllMeshes(const aiScene* pScene)
    {
        for (uint32_t i = 0; i < _meshData.subMeshes.size(); i++)
        {
            const aiMesh* paiMesh = pScene->mMeshes[i];
            initSingleMesh(paiMesh);
        }
    }

    void initSingleMesh(const aiMesh* paiMesh)
    {
        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

        for (uint32_t i = 0; i < paiMesh->mNumVertices; ++i)
        {
            const aiVector3D& pos = paiMesh->mVertices[i];
            const aiVector3D& normal = paiMesh->HasNormals() ? paiMesh->mNormals[i] : Zero3D;
            const aiVector3D& uv = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;

            _meshData.vertices.emplace_back(MeshVertex
            {
                { pos.x, pos.y, pos.z },
                { uv.x, uv.y },
                { normal.x, normal.y, normal.z },
            });
        }

        for (uint32_t i = 0; i < paiMesh->mNumFaces; ++i) 
        {
            const aiFace& Face = paiMesh->mFaces[i];
            assert(Face.mNumIndices == 3);

            _meshData.indicies.push_back(Face.mIndices[0]);
            _meshData.indicies.push_back(Face.mIndices[1]);
            _meshData.indicies.push_back(Face.mIndices[2]);
        }
    }

    bool initMaterials(const aiScene* pScene, const std::filesystem::path& filename)
    {
        const auto dir = filename.has_parent_path() ? filename.parent_path() : std::filesystem::path(".");

        for (uint32_t i = 0; i < pScene->mNumMaterials; i++) 
        {
            const aiMaterial* pMaterial = pScene->mMaterials[i];

            _textures[i].Create();

            if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) 
            {
                aiString Path;

                if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) 
                {
                    std::string p(Path.data);

                    if (p.substr(0, 2) == ".\\") 
                    {
                        p = p.substr(2, p.size() - 2);
                    }

                    const auto full_path = dir / p;

                    printf("full path - %s\n", full_path.string().c_str());

                    auto imgData = _imageLoader.Load(full_path.string());

                    if (!imgData.pixels.get())
                    {
                        printf("Error loading texture '%s'\n", full_path.string().c_str());
                        _textures[i].Destroy();
                        return false;
                    }
                    else 
                    {
                        _textures[i].Upload(imgData);
                        printf("Loaded texture '%s'\n", full_path.string().c_str());
                    }
                }
            }
        }

        return true;
    }

    void populateBuffers()
    {
        _vbo.Storage(Eugenix::Core::MakeData(_meshData.vertices));
        _ibo.Storage(Eugenix::Core::MakeData(_meshData.indicies));

        _vao.AttachVertices(0, _vbo, sizeof(MeshVertex));
        _vao.AttachIndices(_ibo);

        _vao.Attribute({ position_location, 3, Eugenix::Render::DataType::Float, false, offsetof(MeshVertex, position), 0 });
        _vao.Attribute({ tex_coord_location, 2, Eugenix::Render::DataType::Float, false, offsetof(MeshVertex, uv), 0 });
        _vao.Attribute({ normal_location,   3, Eugenix::Render::DataType::Float, false, offsetof(MeshVertex, normal), 0 });

        _meshData.vertices.clear();
        _meshData.indicies.clear();
    }

    WorldTransform _worldTransform;

    Eugenix::Render::OpenGL::VertexArray _vao;
    Eugenix::Render::OpenGL::Buffer _vbo;
    Eugenix::Render::OpenGL::Buffer _ibo;

    std::vector<Eugenix::Render::OpenGL::Texture2D> _textures;

    // Temporary space for vertex stuff before we load them into the GPU
    //std::vector<MeshVertex> _vertices;
    //std::vector<uint32_t> _indices;
    MeshData _meshData{};

    // «десь ему не место?..
    Eugenix::Assets::ImageLoader _imageLoader;
};