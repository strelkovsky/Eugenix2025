#pragma once

#include <map>
#include <string>
#include <vector>

#include "OGLDevMaterial.h"
#include "OGLDevMath.h"

#include "Assets/AssimpModelLoader.h"
#include "Assets/ImageLoader.h"
#include "Render/OpenGL/EugenixGL.h"
#include "Render/OpenGL/Texture2D.h"

namespace
{

}

inline constexpr uint32_t position_location = 0;
inline constexpr uint32_t tex_coord_location = 1;
inline constexpr uint32_t normal_location = 2;
inline constexpr uint32_t bone_id_location = 3;
inline constexpr uint32_t bone_weight_location = 4;

inline constexpr uint32_t invalid_material = std::numeric_limits<uint32_t>::max();
inline constexpr uint32_t max_bones_per_vertex = 4;

struct MeshVertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

struct SkinnedMeshVertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    
    int boneIDs[max_bones_per_vertex];
    float boneWeights[max_bones_per_vertex];

    void AddBoneData(uint32_t boneID, float weight)
    {
        for (uint32_t i = 0; i < max_bones_per_vertex; ++i)
        {
            if (boneWeights[i] == 0.0f)
            {
                boneIDs[i] = boneID;
                boneWeights[i] = weight;
                return;
            }
        }

        uint32_t minIndex = 0;
        for (uint32_t i = 1; i < max_bones_per_vertex; ++i)
        {
            if (boneWeights[i] < boneWeights[minIndex])
                minIndex = i;
        }

        if (weight > boneWeights[minIndex])
        {
            boneIDs[minIndex] = boneID;
            boneWeights[minIndex] = weight;
        }
    }
};

// CPU-mesh

struct SubMesh
{
    uint32_t numIndices{};
    uint32_t baseVertex{};
    uint32_t baseIndex{};
    uint32_t materialIndex{ invalid_material };
};

struct MeshData
{
    std::vector<MeshVertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<SubMesh> subMeshes;
};

// CPU-mesh
struct SkinnedMeshData
{
    std::vector<SkinnedMeshVertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<SubMesh> subMeshes;

    // Или это в StaticMesh классе?
    std::map<std::string, uint32_t> m_BoneNameToIndexMap;
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

        const aiScene* scene = _modelLoader.Load(filename.string());

        if (!scene)
        {
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

            assert(MaterialIndex < _materials.size());

            if (_materials[MaterialIndex].diffuse.NativeHandle() != 0) 
            {
                _materials[MaterialIndex].diffuse.Bind(0);
            }

            if (_materials[MaterialIndex].specularComponent.NativeHandle() != 0)
            {
                _materials[MaterialIndex].specularComponent.Bind(1);
            }

            glDrawElementsBaseVertex(GL_TRIANGLES,
                submesh.numIndices,
                GL_UNSIGNED_INT,
                (void*)(sizeof(uint32_t) * submesh.baseIndex),
                submesh.baseVertex);
        }
    }

    uint32_t NumSubMeshes() const { return _meshData.subMeshes.size(); }

    void RenderSubMesh(uint32_t drawIndex)
    {
        _vao.Bind();

        const auto& submesh = _meshData.subMeshes[drawIndex];

        uint32_t MaterialIndex = submesh.materialIndex;
        assert(MaterialIndex < _materials.size());

        if (_materials[MaterialIndex].diffuse.NativeHandle() != 0)
        {
            _materials[MaterialIndex].diffuse.Bind(0);
        }

        if (_materials[MaterialIndex].specularComponent.NativeHandle() != 0)
        {
            _materials[MaterialIndex].specularComponent.Bind(1);
        }

        glDrawElementsBaseVertex(GL_TRIANGLES,
            submesh.numIndices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(uint32_t) * submesh.baseIndex),
            submesh.baseVertex);
    }

    void Render(unsigned int DrawIndex, unsigned int PrimID)
    {
        assert(DrawIndex < _meshData.subMeshes.size());

        printf("DrawIndex - %d\n", DrawIndex);

        _vao.Bind();

        uint32_t MaterialIndex = _meshData.subMeshes[DrawIndex].materialIndex;

        assert(MaterialIndex < _materials.size());

        if (_materials[MaterialIndex].diffuse.NativeHandle() != 0)
        {
            _materials[MaterialIndex].diffuse.Bind(0);
        }

        if (_materials[MaterialIndex].specularComponent.NativeHandle() != 0)
        {
            _materials[MaterialIndex].specularComponent.Bind(1);
        }

        glDrawElementsBaseVertex(GL_TRIANGLES,
            3,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * (_meshData.subMeshes[DrawIndex].baseIndex + PrimID * 3)),
            _meshData.subMeshes[DrawIndex].baseVertex);
    }

    // ВРЕМЕННЫЙ ХАК
    const OGLDevMaterial& GetMaterial()
    {
        for (unsigned int i = 0; i < _materials.size(); i++) 
        {
            if (_materials[i].ambientColor != glm::vec3(0.0f, 0.0f, 0.0f)) 
            {
                return _materials[i];
            }
        }

        return _materials[0];
    }

    WorldTransform& GetWorldTransform() { return _worldTransform; }

private:
    void clear()
    {
        if (_vbo.NativeHandle() != 0) { _vbo.Destroy(); }
        if (_ibo.NativeHandle() != 0) { _ibo.Destroy(); }
        if (_vao.NativeHandle() != 0) { _vao.Destroy(); }

        _meshData.subMeshes.clear();

        _meshData.vertices.clear();
        _meshData.indices.clear();

        _materials.clear();
    }

    bool initFromScene(const aiScene* pScene, const std::filesystem::path& path)
    {
        _meshData.subMeshes.resize(pScene->mNumMeshes);
        _materials.resize(pScene->mNumMaterials);

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
        _meshData.indices.reserve(numIndices);
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

            _meshData.indices.push_back(Face.mIndices[0]);
            _meshData.indices.push_back(Face.mIndices[1]);
            _meshData.indices.push_back(Face.mIndices[2]);
        }
    }

    bool initMaterials(const aiScene* pScene, const std::filesystem::path& filename)
    {
        const auto dir = filename.has_parent_path() ? filename.parent_path() : std::filesystem::path(".");

        for (uint32_t i = 0; i < pScene->mNumMaterials; i++) 
        {
            const aiMaterial* pMaterial = pScene->mMaterials[i];
            LoadTextures(dir, pMaterial, i);
            LoadColors(pMaterial, i);
        }

        return true;
    }

    void LoadTextures(const std::filesystem::path& dir, const aiMaterial* pMaterial, int index)
    {
        LoadDiffuseTexture(dir, pMaterial, index);
        LoadSpecularTexture(dir, pMaterial, index);
    }
    void LoadDiffuseTexture(const std::filesystem::path& dir, const aiMaterial* pMaterial, int index)
    {
        _materials[index].diffuse = {};

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

                printf("full diffuse path - %s\n", full_path.string().c_str());

                auto imgData = _imageLoader.Load(full_path.string());

                _materials[index].diffuse.Create();

                if (!imgData.pixels.get())
                {
                    printf("Error loading diffuse texture '%s'\n", full_path.string().c_str());
                    _materials[index].diffuse.Destroy();
                }
                else
                {
                    _materials[index].diffuse.Upload(imgData);
                    printf("Loaded diffuse texture '%s'\n", full_path.string().c_str());
                }
            }
        }
    }
    void LoadSpecularTexture(const std::filesystem::path& dir, const aiMaterial* pMaterial, int index)
    {
        _materials[index].specularComponent = {};

        if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0)
        {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
            {
                std::string p(Path.data);

                if (p.substr(0, 2) == ".\\")
                {
                    p = p.substr(2, p.size() - 2);
                }

                const auto full_path = dir / p;

                printf("full specular path - %s\n", full_path.string().c_str());

                auto imgData = _imageLoader.Load(full_path.string());

                _materials[index].specularComponent.Create();

                if (!imgData.pixels.get())
                {
                    printf("Error loading specular texture '%s'\n", full_path.string().c_str());
                    _materials[index].specularComponent.Destroy();
                }
                else
                {
                    _materials[index].specularComponent.Upload(imgData);
                    printf("Loaded specular texture '%s'\n", full_path.string().c_str());
                }
            }
        }
    }

    void LoadColors(const aiMaterial* pMaterial, int index)
    {
        aiColor3D AmbientColor(0.0f, 0.0f, 0.0f);
        if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor) == AI_SUCCESS)
        {
            printf("Loaded ambient color [%f %f %f]\n", AmbientColor.r, AmbientColor.g, AmbientColor.b);
            _materials[index].ambientColor.r = AmbientColor.r;
            _materials[index].ambientColor.g = AmbientColor.g;
            _materials[index].ambientColor.b = AmbientColor.b;
        }
        else
        {
            //fallback для старых/рандомных ассетов
            _materials[index].ambientColor = {1.0f, 1.0f, 1.0f};
        }

        aiColor3D DiffuseColor(0.0f, 0.0f, 0.0f);
        if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS)
        {
            printf("Loaded diffuse color [%f %f %f]\n", DiffuseColor.r, DiffuseColor.g, DiffuseColor.b);
            _materials[index].diffuseColor.r = DiffuseColor.r;
            _materials[index].diffuseColor.g = DiffuseColor.g;
            _materials[index].diffuseColor.b = DiffuseColor.b;
        }
        else
        {
            //fallback для старых/рандомных ассетов
            _materials[index].diffuseColor = { 1.0f, 1.0f, 1.0f };
        }

        aiColor3D SpecularColor(0.0f, 0.0f, 0.0f);
        if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, SpecularColor) == AI_SUCCESS) 
        {
            printf("Loaded specular color [%f %f %f]\n", SpecularColor.r, SpecularColor.g, SpecularColor.b);
            _materials[index].specularColor.r = SpecularColor.r;
            _materials[index].specularColor.g = SpecularColor.g;
            _materials[index].specularColor.b = SpecularColor.b;
        }
        else
        {
            //fallback для старых/рандомных ассетов
            _materials[index].specularColor = { 1.0f, 1.0f, 1.0f };
        }
    }

    void populateBuffers()
    {
        _vbo.Storage(Eugenix::Core::MakeData(_meshData.vertices));
        _ibo.Storage(Eugenix::Core::MakeData(_meshData.indices));

        _vao.AttachVertices(0, _vbo, sizeof(MeshVertex));
        _vao.AttachIndices(_ibo);

        _vao.Attribute({ position_location, 3, Eugenix::Render::DataType::Float, false, offsetof(MeshVertex, position), 0 });
        _vao.Attribute({ tex_coord_location, 2, Eugenix::Render::DataType::Float, false, offsetof(MeshVertex, uv), 0 });
        _vao.Attribute({ normal_location,   3, Eugenix::Render::DataType::Float, false, offsetof(MeshVertex, normal), 0 });

        _meshData.vertices.clear();
        _meshData.indices.clear();
    }

    WorldTransform _worldTransform;

    Eugenix::Render::OpenGL::VertexArray _vao;
    Eugenix::Render::OpenGL::Buffer _vbo;
    Eugenix::Render::OpenGL::Buffer _ibo;

    //std::vector<Eugenix::Render::OpenGL::Texture2D> _textures;
    std::vector<OGLDevMaterial> _materials;

    // Temporary space for vertex stuff before we load them into the GPU
    MeshData _meshData{};

    // Здесь не место?..
    Eugenix::Assets::AssimpModelLoader _modelLoader{};
    Eugenix::Assets::ImageLoader _imageLoader{};
};



// GPU-mesh
class SkinnedMesh
{
public:
    SkinnedMesh() = default;

    SkinnedMesh(const SkinnedMesh&) = delete;
    SkinnedMesh& operator=(const SkinnedMesh&) = delete;

    ~SkinnedMesh()
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

        pScene = _modelLoader.Load(filename.string());

        if (!pScene)
        {
            clear();
            return false;
        }

        printf("animations: %u\n", pScene->mNumAnimations);

        m_GlobalInverseTransform = AssimpToGlm(pScene->mRootNode->mTransformation);
        m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);

        if (!initFromScene(pScene, filename))
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

            assert(MaterialIndex < _materials.size());

            if (_materials[MaterialIndex].diffuse.NativeHandle() != 0)
            {
                _materials[MaterialIndex].diffuse.Bind(0);
            }

            if (_materials[MaterialIndex].specularComponent.NativeHandle() != 0)
            {
                _materials[MaterialIndex].specularComponent.Bind(1);
            }

            glDrawElementsBaseVertex(GL_TRIANGLES,
                submesh.numIndices,
                GL_UNSIGNED_INT,
                (void*)(sizeof(uint32_t) * submesh.baseIndex),
                submesh.baseVertex);
        }
    }

    // ВРЕМЕННЫЙ ХАК
    const OGLDevMaterial& GetMaterial()
    {
        for (unsigned int i = 0; i < _materials.size(); i++)
        {
            if (_materials[i].ambientColor != glm::vec3(0.0f, 0.0f, 0.0f))
            {
                return _materials[i];
            }
        }

        return _materials[0];
    }

    WorldTransform& GetWorldTransform() { return _worldTransform; }

    uint32_t NumBones() const
    {
        return _meshData.m_BoneNameToIndexMap.size();
    }

    void GetBoneTransforms(float TimeInSeconds, std::vector<glm::mat4>& transforms)
    {
        transforms.resize(m_BoneInfo.size());

        glm::mat4 identity{ 1.0f };

        float TicksPerSecond = (float)(pScene->mAnimations[0]->mTicksPerSecond != 0 ? pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
        float TimeInTicks = TimeInSeconds * TicksPerSecond;
        float AnimationTimeTicks = fmod(TimeInTicks, (float)pScene->mAnimations[0]->mDuration);

        ReadNodeHierarchy(AnimationTimeTicks, pScene->mRootNode, identity);

        for (uint32_t i = 0; i < m_BoneInfo.size(); i++)
        {
            transforms[i] = m_BoneInfo[i].finalTransfomration;
        }
    }

private:
    void clear()
    {
        if (_vbo.NativeHandle() != 0) { _vbo.Destroy(); }
        if (_ibo.NativeHandle() != 0) { _ibo.Destroy(); }
        if (_vao.NativeHandle() != 0) { _vao.Destroy(); }

        _meshData.subMeshes.clear();

        _meshData.vertices.clear();
        _meshData.indices.clear();

        _meshData.m_BoneNameToIndexMap.clear();

        _materials.clear();

        m_BoneInfo.clear();
        pScene = nullptr;
    }

    bool initFromScene(const aiScene* pScene, const std::filesystem::path& path)
    {
        _meshData.subMeshes.resize(pScene->mNumMeshes);
        _materials.resize(pScene->mNumMaterials);

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
        _meshData.indices.reserve(numIndices);
    }

    void initAllMeshes(const aiScene* pScene)
    {
        for (uint32_t i = 0; i < _meshData.subMeshes.size(); i++)
        {
            const aiMesh* paiMesh = pScene->mMeshes[i];
            initSingleMesh(i, paiMesh);
        }
    }

    void initSingleMesh(uint32_t MeshIndex, const aiMesh* paiMesh)
    {
        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
        const aiVector3D DefaultNormal(0.0f, 1.0f, 0.0f);

        for (uint32_t i = 0; i < paiMesh->mNumVertices; ++i)
        {
            const aiVector3D& pos = paiMesh->mVertices[i];
            const aiVector3D& normal = paiMesh->HasNormals() ? paiMesh->mNormals[i] : DefaultNormal;
            const aiVector3D& uv = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;

            _meshData.vertices.emplace_back(SkinnedMeshVertex
                {
                    { pos.x, pos.y, pos.z },
                    { uv.x, uv.y },
                    { normal.x, normal.y, normal.z },
                    { 0, 0, 0, 0 },
                    //{ -1, -1, -1, -1 },
                    { 0.0f, 0.0f, 0.0f, 0.0f }
                });
        }

        loadMeshBones(MeshIndex, paiMesh);

        for (uint32_t i = 0; i < paiMesh->mNumFaces; ++i)
        {
            const aiFace& Face = paiMesh->mFaces[i];
            assert(Face.mNumIndices == 3);

            _meshData.indices.push_back(Face.mIndices[0]);
            _meshData.indices.push_back(Face.mIndices[1]);
            _meshData.indices.push_back(Face.mIndices[2]);
        }
    }

    void loadMeshBones(uint32_t MeshIndex, const aiMesh* pMesh)
    {
        for (uint32_t i = 0; i < pMesh->mNumBones; i++) 
        {
            loadSingleBone(MeshIndex, pMesh->mBones[i]);
        }

        // Нормализация весов.
        for (auto& v : _meshData.vertices)
        {
            float sum = 0.0f;
            for (float w : v.boneWeights)
                sum += w;

            if (sum > 0.0f)
            {
                for (float& w : v.boneWeights)
                    w /= sum;
            }
        }
    }

    void loadSingleBone(uint32_t MeshIndex, const aiBone* pBone)
    {
        int BoneId = getBoneId(pBone);

        if (BoneId == m_BoneInfo.size())
        {
            BoneInfo bi(AssimpToGlm(pBone->mOffsetMatrix));
            m_BoneInfo.push_back(bi);
        }

        for (uint32_t i = 0; i < pBone->mNumWeights; i++) 
        {
            const aiVertexWeight& vw = pBone->mWeights[i];
            uint32_t GlobalVertexID = _meshData.subMeshes[MeshIndex].baseVertex + pBone->mWeights[i].mVertexId;
            _meshData.vertices[GlobalVertexID].AddBoneData(BoneId, vw.mWeight);
        }
    }

    int getBoneId(const aiBone* pBone)
    {
        int BoneIndex = 0;
        std::string BoneName(pBone->mName.C_Str());

        if (_meshData.m_BoneNameToIndexMap.find(BoneName) == _meshData.m_BoneNameToIndexMap.end())
        {
            // Allocate an index for a new bone
            BoneIndex = _meshData.m_BoneNameToIndexMap.size();
            _meshData.m_BoneNameToIndexMap[BoneName] = BoneIndex;
        }
        else 
        {
            BoneIndex = _meshData.m_BoneNameToIndexMap[BoneName];
        }

        return BoneIndex;
    }

    static glm::mat4 AssimpToGlm(const aiMatrix4x4& m)
    {
        glm::mat4 result{ 1.0f };

        result[0][0] = m.a1; result[1][0] = m.a2; result[2][0] = m.a3; result[3][0] = m.a4;
        result[0][1] = m.b1; result[1][1] = m.b2; result[2][1] = m.b3; result[3][1] = m.b4;
        result[0][2] = m.c1; result[1][2] = m.c2; result[2][2] = m.c3; result[3][2] = m.c4;
        result[0][3] = m.d1; result[1][3] = m.d2; result[2][3] = m.d3; result[3][3] = m.d4;

        //result[0][0] = m.a1; result[0][1] = m.b1; result[0][2] = m.c1; result[0][3] = m.d1;
        //result[1][0] = m.a2; result[1][1] = m.b2; result[1][2] = m.c2; result[1][3] = m.d2;
        //result[2][0] = m.a3; result[2][1] = m.b3; result[2][2] = m.c3; result[2][3] = m.d3;
        //result[3][0] = m.a4; result[3][1] = m.b4; result[3][2] = m.c4; result[3][3] = m.d4;

        return result;
    }

    static glm::quat AssimpToGlm(const aiQuaternion& quat)
    {
        return glm::quat{ quat.w, quat.x, quat.y, quat.z };
    }

    void CalcInterpolatedPosition(aiVector3D& out, float t, const aiNodeAnim* ch)
    {
        assert(ch);

        if (ch->mNumPositionKeys == 0)
        {
            out = aiVector3D(0.0f, 0.0f, 0.0f);
            return;
        }

        if (ch->mNumPositionKeys == 1)
        {
            out = ch->mPositionKeys[0].mValue;
            return;
        }

        for (uint32_t i = 0; i + 1 < ch->mNumPositionKeys; ++i)
        {
            const float t1 = static_cast<float>(ch->mPositionKeys[i].mTime);
            const float t2 = static_cast<float>(ch->mPositionKeys[i + 1].mTime);

            if (t < t2)
            {
                const float delta = t2 - t1;
                if (delta <= 1e-6f)
                {
                    out = ch->mPositionKeys[i].mValue;
                    return;
                }

                float factor = (t - t1) / delta;
                factor = std::clamp(factor, 0.0f, 1.0f);

                const aiVector3D& a = ch->mPositionKeys[i].mValue;
                const aiVector3D& b = ch->mPositionKeys[i + 1].mValue;

                out = a + (b - a) * factor;
                return;
            }
        }

        out = ch->mPositionKeys[ch->mNumPositionKeys - 1].mValue;
    }

    void CalcInterpolatedRotation(aiQuaternion& out, float t, const aiNodeAnim* ch)
    {
        assert(ch);

        if (ch->mNumRotationKeys == 0)
        {
            out = aiQuaternion();
            return;
        }

        if (ch->mNumRotationKeys == 1)
        {
            out = ch->mRotationKeys[0].mValue;
            out.Normalize();
            return;
        }

        for (uint32_t i = 0; i + 1 < ch->mNumRotationKeys; ++i)
        {
            const float t1 = static_cast<float>(ch->mRotationKeys[i].mTime);
            const float t2 = static_cast<float>(ch->mRotationKeys[i + 1].mTime);

            if (t < t2)
            {
                const float delta = t2 - t1;
                if (delta <= 1e-6f)
                {
                    out = ch->mRotationKeys[i].mValue;
                    out.Normalize();
                    return;
                }

                float factor = (t - t1) / delta;
                factor = std::clamp(factor, 0.0f, 1.0f);

                const aiQuaternion& a = ch->mRotationKeys[i].mValue;
                const aiQuaternion& b = ch->mRotationKeys[i + 1].mValue;

                aiQuaternion::Interpolate(out, a, b, factor);
                out.Normalize();
                return;
            }
        }

        out = ch->mRotationKeys[ch->mNumRotationKeys - 1].mValue;
        out.Normalize();
    }

    void CalcInterpolatedScaling(aiVector3D& out, float t, const aiNodeAnim* ch)
    {
        assert(ch);

        if (ch->mNumScalingKeys == 0)
        {
            out = aiVector3D(1.0f, 1.0f, 1.0f);
            return;
        }

        if (ch->mNumScalingKeys == 1)
        {
            out = ch->mScalingKeys[0].mValue;
            return;
        }

        for (uint32_t i = 0; i + 1 < ch->mNumScalingKeys; ++i)
        {
            const float t1 = static_cast<float>(ch->mScalingKeys[i].mTime);
            const float t2 = static_cast<float>(ch->mScalingKeys[i + 1].mTime);

            if (t < t2)
            {
                const float delta = t2 - t1;
                if (delta <= 1e-6f)
                {
                    out = ch->mScalingKeys[i].mValue;
                    return;
                }

                float factor = (t - t1) / delta;
                factor = std::clamp(factor, 0.0f, 1.0f);

                const aiVector3D& a = ch->mScalingKeys[i].mValue;
                const aiVector3D& b = ch->mScalingKeys[i + 1].mValue;

                out = a + (b - a) * factor;
                return;
            }
        }

        out = ch->mScalingKeys[ch->mNumScalingKeys - 1].mValue;
    }

    const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string& NodeName)
    {
        for (uint32_t i = 0; i < pAnimation->mNumChannels; i++) 
        {
            const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

            if (std::string(pNodeAnim->mNodeName.data) == NodeName) 
            {
                return pNodeAnim;
            }
        }

        return NULL;
    }

    static void PrintMat4(const char* name, const glm::mat4& m)
    {
        printf("%s:\n", name);
        for (int r = 0; r < 4; ++r)
        {
            printf("  [% .6f % .6f % .6f % .6f]\n",
                m[0][r], m[1][r], m[2][r], m[3][r]);
        }
    }

    static bool IsFiniteMat4(const glm::mat4& m)
    {
        for (int c = 0; c < 4; ++c)
        {
            for (int r = 0; r < 4; ++r)
            {
                if (!std::isfinite(m[c][r]))
                {
                    return false;
                }
            }
        }
        return true;
    }

    static bool ShouldAnimateTranslation(const std::string& nodeName)
    {
        return nodeName == "origin" || nodeName == "pelvis";
    }

    void ReadNodeHierarchy(float animationTimeTicks, const aiNode* pNode, const glm::mat4& parentTransform)
    {
        const std::string nodeName(pNode->mName.C_Str());
        const aiAnimation* animation = pScene->mAnimations[0];

        aiVector3D bindScaling;
        aiQuaternion bindRotation;
        aiVector3D bindTranslation;
        pNode->mTransformation.Decompose(bindScaling, bindRotation, bindTranslation);

        aiVector3D scaling = bindScaling;
        aiQuaternion rotationQ = bindRotation;
        aiVector3D translation = bindTranslation;

        const aiNodeAnim* nodeAnim = FindNodeAnim(animation, nodeName);
        if (nodeAnim)
        {
            if (nodeAnim->mNumRotationKeys > 0)
            {
                CalcInterpolatedRotation(rotationQ, animationTimeTicks, nodeAnim);
            }

            if (ShouldAnimateTranslation(nodeName) && nodeAnim->mNumPositionKeys > 0)
            {
                CalcInterpolatedPosition(translation, animationTimeTicks, nodeAnim);
            }

            // scale пока намеренно не анимируем
        }

        aiMatrix4x4 scalingM;
        aiMatrix4x4::Scaling(scaling, scalingM);

        aiMatrix4x4 rotationM(rotationQ.GetMatrix());

        aiMatrix4x4 translationM;
        aiMatrix4x4::Translation(translation, translationM);

        aiMatrix4x4 nodeTransformationAi = translationM * rotationM * scalingM;

        glm::mat4 nodeTransformation = AssimpToGlm(nodeTransformationAi);
        glm::mat4 globalTransformation = parentTransform * nodeTransformation;

        auto it = _meshData.m_BoneNameToIndexMap.find(nodeName);
        if (it != _meshData.m_BoneNameToIndexMap.end())
        {
            const uint32_t boneIndex = it->second;
            m_BoneInfo[boneIndex].finalTransfomration =
                m_GlobalInverseTransform * globalTransformation * m_BoneInfo[boneIndex].offsetMatrix;
        }

        for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
        {
            ReadNodeHierarchy(animationTimeTicks, pNode->mChildren[i], globalTransformation);
        }
    }

    bool initMaterials(const aiScene* pScene, const std::filesystem::path& filename)
    {
        const auto dir = filename.has_parent_path() ? filename.parent_path() : std::filesystem::path(".");

        for (uint32_t i = 0; i < pScene->mNumMaterials; i++)
        {
            const aiMaterial* pMaterial = pScene->mMaterials[i];
            LoadTextures(dir, pMaterial, i);
            LoadColors(pMaterial, i);
        }

        return true;
    }

    void LoadTextures(const std::filesystem::path& dir, const aiMaterial* pMaterial, int index)
    {
        LoadDiffuseTexture(dir, pMaterial, index);
        LoadSpecularTexture(dir, pMaterial, index);
    }
    void LoadDiffuseTexture(const std::filesystem::path& dir, const aiMaterial* pMaterial, int index)
    {
        _materials[index].diffuse = {};

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

                printf("full diffuse path - %s\n", full_path.string().c_str());

                auto imgData = _imageLoader.Load(full_path.string());

                _materials[index].diffuse.Create();

                if (!imgData.pixels.get())
                {
                    printf("Error loading diffuse texture '%s'\n", full_path.string().c_str());
                    _materials[index].diffuse.Destroy();
                }
                else
                {
                    _materials[index].diffuse.Upload(imgData);
                    printf("Loaded diffuse texture '%s'\n", full_path.string().c_str());
                }
            }
        }
    }
    void LoadSpecularTexture(const std::filesystem::path& dir, const aiMaterial* pMaterial, int index)
    {
        _materials[index].specularComponent = {};

        if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0)
        {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
            {
                std::string p(Path.data);

                if (p.substr(0, 2) == ".\\")
                {
                    p = p.substr(2, p.size() - 2);
                }

                const auto full_path = dir / p;

                printf("full specular path - %s\n", full_path.string().c_str());

                auto imgData = _imageLoader.Load(full_path.string());

                _materials[index].specularComponent.Create();

                if (!imgData.pixels.get())
                {
                    printf("Error loading specular texture '%s'\n", full_path.string().c_str());
                    _materials[index].specularComponent.Destroy();
                }
                else
                {
                    _materials[index].specularComponent.Upload(imgData);
                    printf("Loaded specular texture '%s'\n", full_path.string().c_str());
                }
            }
        }
    }

    void LoadColors(const aiMaterial* pMaterial, int index)
    {
        aiColor3D AmbientColor(0.0f, 0.0f, 0.0f);
        glm::vec3 AllOnes{1.0f, 1.0f, 1.0f};

        int ShadingModel = 0;
        if (pMaterial->Get(AI_MATKEY_SHADING_MODEL, ShadingModel) == AI_SUCCESS) 
        {
            printf("Shading model %d\n", ShadingModel);
        }

        if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor) == AI_SUCCESS)
        {
            printf("Loaded ambient color [%f %f %f]\n", AmbientColor.r, AmbientColor.g, AmbientColor.b);
            _materials[index].ambientColor.r = AmbientColor.r;
            _materials[index].ambientColor.g = AmbientColor.g;
            _materials[index].ambientColor.b = AmbientColor.b;
        }
        else
        {
            _materials[index].ambientColor = AllOnes;
        }

        aiColor3D DiffuseColor(0.0f, 0.0f, 0.0f);
        if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS)
        {
            printf("Loaded diffuse color [%f %f %f]\n", DiffuseColor.r, DiffuseColor.g, DiffuseColor.b);
            _materials[index].diffuseColor.r = DiffuseColor.r;
            _materials[index].diffuseColor.g = DiffuseColor.g;
            _materials[index].diffuseColor.b = DiffuseColor.b;
        }
        else
        {
            _materials[index].diffuseColor = AllOnes;
        }

        aiColor3D SpecularColor(0.0f, 0.0f, 0.0f);
        if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, SpecularColor) == AI_SUCCESS) {
            printf("Loaded specular color [%f %f %f]\n", SpecularColor.r, SpecularColor.g, SpecularColor.b);
            _materials[index].specularColor.r = SpecularColor.r;
            _materials[index].specularColor.g = SpecularColor.g;
            _materials[index].specularColor.b = SpecularColor.b;
        }
        else
        {
            _materials[index].specularColor = AllOnes;
        }
    }

    void populateBuffers()
    {
        _vbo.Storage(Eugenix::Core::MakeData(_meshData.vertices));
        _ibo.Storage(Eugenix::Core::MakeData(_meshData.indices));

        _vao.AttachVertices(0, _vbo, sizeof(SkinnedMeshVertex));
        _vao.AttachIndices(_ibo);

        _vao.Attribute({ position_location, 3, Eugenix::Render::DataType::Float, false, offsetof(SkinnedMeshVertex, position), 0 });
        _vao.Attribute({ tex_coord_location, 2, Eugenix::Render::DataType::Float, false, offsetof(SkinnedMeshVertex, uv), 0 });
        _vao.Attribute({ normal_location,   3, Eugenix::Render::DataType::Float, false, offsetof(SkinnedMeshVertex, normal), 0 });
        _vao.Attribute({ bone_id_location,   max_bones_per_vertex, Eugenix::Render::DataType::Int, false, offsetof(SkinnedMeshVertex, boneIDs), 0 });
        _vao.Attribute({ bone_weight_location,   max_bones_per_vertex, Eugenix::Render::DataType::Float, false, offsetof(SkinnedMeshVertex, boneWeights), 0 });

        _meshData.vertices.clear();
        _meshData.indices.clear();
    }

    WorldTransform _worldTransform;

    Eugenix::Render::OpenGL::VertexArray _vao;
    Eugenix::Render::OpenGL::Buffer _vbo;
    Eugenix::Render::OpenGL::Buffer _ibo;

    const aiScene* pScene = NULL;
    std::vector<OGLDevMaterial> _materials;

    struct BoneInfo
    {
        glm::mat4 offsetMatrix;
        glm::mat4 finalTransfomration;

        BoneInfo(const glm::mat4& offset)
        {
            offsetMatrix = offset;
            finalTransfomration = glm::mat4{ 0.0f };
        }
    };

    // Temporary space for vertex stuff before we load them into the GPU
    SkinnedMeshData _meshData{};
    glm::mat4 m_GlobalInverseTransform;
    // Здесь или в MeshData?
    std::vector<BoneInfo> m_BoneInfo;

    // Здесь не место?..
    Eugenix::Assets::AssimpModelLoader _modelLoader{};
    Eugenix::Assets::ImageLoader _imageLoader{};
};