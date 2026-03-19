#pragma once

#include <map>
#include <string>
#include <vector>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include "OGLDevMath.h"

#include "Assets/ImageLoader.h"
#include "Render/OpenGL/EugenixGL.h"
#include "Render/OpenGL/Texture2D.h"

static constexpr uint32_t position_location = 0;
static constexpr uint32_t tex_coord_location = 1;
static constexpr uint32_t normal_location = 2;

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices)

#define COLOR_TEXTURE_UNIT              GL_TEXTURE0
#define COLOR_TEXTURE_UNIT_INDEX        0
#define SHADOW_TEXTURE_UNIT             GL_TEXTURE1
#define SHADOW_TEXTURE_UNIT_INDEX       1
#define NORMAL_TEXTURE_UNIT             GL_TEXTURE2
#define NORMAL_TEXTURE_UNIT_INDEX       2
#define RANDOM_TEXTURE_UNIT             GL_TEXTURE3
#define RANDOM_TEXTURE_UNIT_INDEX       3
#define DISPLACEMENT_TEXTURE_UNIT       GL_TEXTURE4
#define DISPLACEMENT_TEXTURE_UNIT_INDEX 4
#define MOTION_TEXTURE_UNIT             GL_TEXTURE5
#define MOTION_TEXTURE_UNIT_INDEX       5

class BasicMesh
{
public:
    BasicMesh() = default;

    BasicMesh(const BasicMesh&) = delete;
    BasicMesh& operator=(const BasicMesh&) = delete;

    ~BasicMesh()
    {
        Clear();
    }

    bool LoadMesh(const std::filesystem::path& filename)
    {
        Clear();

        _vao.Create();
        for (auto& buffer : _buffers)
        {
            buffer.Create();
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(filename.string(), ASSIMP_LOAD_FLAGS);

        if (!scene)
        {
            printf("Error parsing '%s': '%s'\n", filename.string(), importer.GetErrorString());
            Clear();
            return false;
        }

        if (!InitFromScene(scene, filename.string()))
        {
            Clear();
            return false;
        }

        return true;
    }

    void Render()
    {
        _vao.Bind();

        for (uint32_t i = 0; i < _subMeshes.size(); i++) 
        {
            uint32_t MaterialIndex = _subMeshes[i].materialIndex;

            assert(MaterialIndex < m_Textures.size());

            if (m_Textures[MaterialIndex].NativeHandle() != 0) 
            {
                m_Textures[MaterialIndex].Bind(0);
            }

            glDrawElementsBaseVertex(GL_TRIANGLES,
                _subMeshes[i].numIndices,
                GL_UNSIGNED_INT,
                (void*)(sizeof(uint32_t) * _subMeshes[i].baseIndex),
                _subMeshes[i].baseVertex);
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

        //    assert(MaterialIndex < m_Textures.size());

        //    if (m_Textures[MaterialIndex].NativeHandle() > 0) 
        //    {
        //        m_Textures[MaterialIndex].Bind(GL_TEXTURE0);
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
    void Clear()
    {
        for (auto& texture : m_Textures)
        {
            if (texture.NativeHandle() != 0)
            {
                texture.Destroy();
            }
        }

        for (auto& buffer : _buffers)
        {
            if (buffer.NativeHandle() != 0)
            {
                buffer.Destroy();
            }
        }

        if (_vao.NativeHandle() != 0)
        {
            _vao.Destroy();
        }

        _subMeshes.clear();
        m_Textures.clear();

        _positions.clear();
        _normals.clear();
        _uvs.clear();
        _indices.clear();
    }

    bool InitFromScene(const aiScene* pScene, const std::string& Filename)
    {
        _subMeshes.resize(pScene->mNumMeshes);
        m_Textures.resize(pScene->mNumMaterials);

        uint32_t numVertices = 0;
        uint32_t numIndices = 0;

        CountVerticesAndIndices(pScene, numVertices, numIndices);

        ReserveSpace(numVertices, numIndices);

        InitAllMeshes(pScene);

        if (!InitMaterials(pScene, Filename)) 
        {
            return false;
        }

        PopulateBuffers();

        return true;
    }

    void CountVerticesAndIndices(const aiScene* pScene, uint32_t& NumVertices, uint32_t& NumIndices)
    {
        for (uint32_t i = 0; i < _subMeshes.size(); i++)
        {
            _subMeshes[i].materialIndex = pScene->mMeshes[i]->mMaterialIndex;
            _subMeshes[i].numIndices = pScene->mMeshes[i]->mNumFaces * 3;
            _subMeshes[i].baseVertex = NumVertices;
            _subMeshes[i].baseIndex = NumIndices;

            NumVertices += pScene->mMeshes[i]->mNumVertices;
            NumIndices += _subMeshes[i].numIndices;
        }
    }

    void ReserveSpace(uint32_t numVertices, uint32_t numIndices)
    {
        _positions.reserve(numVertices);
        _normals.reserve(numVertices);
        _uvs.reserve(numVertices);
        _indices.reserve(numIndices);
    }

    void InitAllMeshes(const aiScene* pScene)
    {
        for (uint32_t i = 0; i < _subMeshes.size(); i++) 
        {
            const aiMesh* paiMesh = pScene->mMeshes[i];
            InitSingleMesh(paiMesh);
        }
    }

    void InitSingleMesh(const aiMesh* paiMesh)
    {
        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

        for (uint32_t i = 0; i < paiMesh->mNumVertices; ++i)
        {
            const aiVector3D& pos = paiMesh->mVertices[i];
            const aiVector3D& normal = paiMesh->HasNormals() ? paiMesh->mNormals[i] : Zero3D;
            const aiVector3D& uv = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;

            _positions.emplace_back(pos.x, pos.y, pos.z);
            _normals.emplace_back(normal.x, normal.y, normal.z);
            _uvs.emplace_back(uv.x, uv.y);
        }

        for (uint32_t i = 0; i < paiMesh->mNumFaces; ++i) 
        {
            const aiFace& Face = paiMesh->mFaces[i];
            assert(Face.mNumIndices == 3);

            _indices.push_back(Face.mIndices[0]);
            _indices.push_back(Face.mIndices[1]);
            _indices.push_back(Face.mIndices[2]);
        }
    }

    bool InitMaterials(const aiScene* pScene, const std::filesystem::path& filename)
    {
        const auto dir = filename.has_parent_path() ? filename.parent_path() : std::filesystem::path(".");

        for (uint32_t i = 0; i < pScene->mNumMaterials; i++) 
        {
            const aiMaterial* pMaterial = pScene->mMaterials[i];

            m_Textures[i].Create();

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

                    printf("full path - %s\n", full_path.c_str());

                    auto imgData = _imageLoader.Load(full_path.string());

                    m_Textures[i].Upload(imgData);

                    if (!imgData.pixels.get())
                    {
                        printf("Error loading texture '%s'\n", full_path.string().c_str());
                        m_Textures[i].Destroy();
                        return false;
                    }
                    else 
                    {
                        printf("Loaded texture '%s'\n", full_path.string().c_str());
                    }
                }
            }
        }

        return true;
    }

    void PopulateBuffers()
    {
        _buffers[POS_VB].Storage(Eugenix::Core::MakeData(_positions));
        _buffers[TEXCOORD_VB].Storage(Eugenix::Core::MakeData(_uvs));
        _buffers[NORMAL_VB].Storage(Eugenix::Core::MakeData(_normals));
        _buffers[INDEX_BUFFER].Storage(Eugenix::Core::MakeData(_indices));

        _vao.AttachVertices(0, _buffers[POS_VB], sizeof(_positions[0]));
        _vao.AttachVertices(1, _buffers[TEXCOORD_VB], sizeof(_uvs[0]));
        _vao.AttachVertices(2, _buffers[NORMAL_VB], sizeof(_normals[0]));
        _vao.AttachIndices(_buffers[INDEX_BUFFER]);

        _vao.Attribute({ position_location,  3, Eugenix::Render::DataType::Float, false, 0, 0 });
        _vao.Attribute({ tex_coord_location, 2, Eugenix::Render::DataType::Float, false, 0, 1 });
        _vao.Attribute({ normal_location,    3, Eugenix::Render::DataType::Float, false, 0, 2 });

        _positions.clear();
        _normals.clear();
        _uvs.clear();
        _indices.clear();
    }

#define INVALID_MATERIAL 0xFFFFFFFF

    enum BUFFER_TYPE 
    {
        INDEX_BUFFER = 0,
        POS_VB = 1,
        TEXCOORD_VB = 2,
        NORMAL_VB = 3,
        WVP_MAT_VB = 4,
        WORLD_MAT_VB = 5,
        NUM_BUFFERS = 6
    };

    WorldTransform _worldTransform;

    Eugenix::Render::OpenGL::VertexArray _vao;
    Eugenix::Render::OpenGL::Buffer _buffers[NUM_BUFFERS] = {};

    struct SubMesh
    {
        uint32_t numIndices{};
        uint32_t baseVertex{};
        uint32_t baseIndex{};
        uint32_t materialIndex{ INVALID_MATERIAL };
    };

    std::vector<SubMesh> _subMeshes;
    std::vector<Eugenix::Render::OpenGL::Texture2D> m_Textures;
    Eugenix::Render::OpenGL::Sampler _sampler;

    // Temporary space for vertex stuff before we load them into the GPU
    std::vector<glm::vec3> _positions;
    std::vector<glm::vec3> _normals;
    std::vector<glm::vec2> _uvs;
    std::vector<uint32_t> _indices;

    // «десь ему не место?..
    Eugenix::Assets::ImageLoader _imageLoader;
};