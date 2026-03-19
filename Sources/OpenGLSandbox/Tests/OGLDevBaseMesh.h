/*

        Copyright 2011 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OGLDEV_BASIC_MESH_H
#define OGLDEV_BASIC_MESH_H

#include <map>
#include <string>
#include <vector>
//#include <GL/glew.h>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

//#include "ogldev_util.h"
//#include "ogldev_math_3d.h"
//#include "ogldev_texture.h"
//#include "ogldev_world_transform.h"

#include "OGLDevMath.h"

#include "Assets/ImageLoader.h"
#include "Render/OpenGL/EugenixGL.h"
#include "Render/OpenGL/Texture2D.h"

#define POSITION_LOCATION  0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION    2

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices)

#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }

#define GLCheckError() (glGetError() == GL_NO_ERROR)

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
    BasicMesh() {};

    ~BasicMesh()
    {
        Clear();
    }

    bool LoadMesh(const std::string& Filename)
    {
        Clear();

        _vao.Create();

        for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(_buffers); i++)
        {
            _buffers[i].Create();
        }

        bool Ret = false;
        Assimp::Importer Importer;

        const aiScene* pScene = Importer.ReadFile(Filename.c_str(), ASSIMP_LOAD_FLAGS);

        if (pScene)
        {
            Ret = InitFromScene(pScene, Filename);
        }
        else
        {
            printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
        }

        // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);

        return Ret;
    }

    void Render()
    {
        _vao.Bind();

        _sampler.Bind(0);

        for (unsigned int i = 0; i < m_Meshes.size(); i++) 
        {
            unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;

            assert(MaterialIndex < m_Textures.size());

            if (m_Textures[MaterialIndex].NativeHandle() != 0) 
            {
                m_Textures[MaterialIndex].Bind(0);
            }

            glDrawElementsBaseVertex(GL_TRIANGLES,
                m_Meshes[i].NumIndices,
                GL_UNSIGNED_INT,
                (void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
                m_Meshes[i].BaseVertex);
        }
    }

    void Render(unsigned int NumInstances, const glm::mat4* WVPMats, const glm::mat4* WorldMats)
    {
        //glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WVP_MAT_VB]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * NumInstances, WVPMats, GL_DYNAMIC_DRAW);

        //glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WORLD_MAT_VB]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * NumInstances, WorldMats, GL_DYNAMIC_DRAW);

        //glBindVertexArray(m_VAO);

        //for (unsigned int i = 0; i < m_Meshes.size(); i++) 
        //{
        //    const unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;

        //    assert(MaterialIndex < m_Textures.size());

        //    if (m_Textures[MaterialIndex].NativeHandle() > 0) 
        //    {
        //        m_Textures[MaterialIndex].Bind(GL_TEXTURE0);
        //    }

        //    glDrawElementsInstancedBaseVertex(GL_TRIANGLES,
        //        m_Meshes[i].NumIndices,
        //        GL_UNSIGNED_INT,
        //        (void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
        //        NumInstances,
        //        m_Meshes[i].BaseVertex);
        //}

        //// Make sure the VAO is not changed from the outside
        //glBindVertexArray(0);
    }

    WorldTransform& GetWorldTransform() { return m_worldTransform; }

private:
    void Clear()
    {
        for (unsigned int i = 0; i < m_Textures.size(); i++) 
        {
            m_Textures[i].Destroy();
        }

        for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(_buffers); i++)
        {
            _buffers[i].Destroy();
        }

        if (_vao.NativeHandle() != 0)
        {
            _vao.Destroy();
        }
    }

    bool InitFromScene(const aiScene* pScene, const std::string& Filename)
    {
        m_Meshes.resize(pScene->mNumMeshes);
        m_Textures.resize(pScene->mNumMaterials);

        unsigned int NumVertices = 0;
        unsigned int NumIndices = 0;

        CountVerticesAndIndices(pScene, NumVertices, NumIndices);

        ReserveSpace(NumVertices, NumIndices);

        InitAllMeshes(pScene);

        if (!InitMaterials(pScene, Filename)) 
        {
            return false;
        }

        PopulateBuffers();

        return GLCheckError();
    }

    void CountVerticesAndIndices(const aiScene* pScene, unsigned int& NumVertices, unsigned int& NumIndices)
    {
        for (unsigned int i = 0; i < m_Meshes.size(); i++) 
        {
            m_Meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
            m_Meshes[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
            m_Meshes[i].BaseVertex = NumVertices;
            m_Meshes[i].BaseIndex = NumIndices;

            NumVertices += pScene->mMeshes[i]->mNumVertices;
            NumIndices += m_Meshes[i].NumIndices;
        }
    }

    void ReserveSpace(unsigned int NumVertices, unsigned int NumIndices)
    {
        m_Positions.reserve(NumVertices);
        m_Normals.reserve(NumVertices);
        m_TexCoords.reserve(NumVertices);
        m_Indices.reserve(NumIndices);
    }

    void InitAllMeshes(const aiScene* pScene)
    {
        for (unsigned int i = 0; i < m_Meshes.size(); i++) 
        {
            const aiMesh* paiMesh = pScene->mMeshes[i];
            InitSingleMesh(paiMesh);
        }
    }

    void InitSingleMesh(const aiMesh* paiMesh)
    {
        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

        // Populate the vertex attribute vectors
        for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) 
        {
            const aiVector3D& pPos = paiMesh->mVertices[i];
            const aiVector3D& pNormal = paiMesh->mNormals[i];
            const aiVector3D& pTexCoord = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;

            m_Positions.push_back(glm::vec3(pPos.x, pPos.y, pPos.z));
            m_Normals.push_back(glm::vec3(pNormal.x, pNormal.y, pNormal.z));
            m_TexCoords.push_back(glm::vec2(pTexCoord.x, pTexCoord.y));
        }

        // Populate the index buffer
        for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) 
        {
            const aiFace& Face = paiMesh->mFaces[i];
            assert(Face.mNumIndices == 3);
            m_Indices.push_back(Face.mIndices[0]);
            m_Indices.push_back(Face.mIndices[1]);
            m_Indices.push_back(Face.mIndices[2]);
        }
    }

    bool InitMaterials(const aiScene* pScene, const std::string& Filename)
    {
        // Extract the directory part from the file name
        size_t SlashIndex = Filename.find_last_of("/");
        std::string Dir;

        if (SlashIndex == -1) 
        {
            Dir = ".";
        }
        else if (SlashIndex == 0) 
        {
            Dir = "/";
        }
        else 
        {
            Dir = Filename.substr(0, SlashIndex);
        }

        bool Ret = true;

        // Initialize the materials
        for (unsigned int i = 0; i < pScene->mNumMaterials; i++) 
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

                    std::string FullPath = Dir + "/" + p;

                    printf("full path - %s\n", FullPath.c_str());

                    auto imgData = _imageLoader.Load(FullPath);

                    m_Textures[i].Upload(imgData);

                    if (!imgData.pixels.get())
                    {
                        printf("Error loading texture '%s'\n", FullPath.c_str());
                        m_Textures[i].Destroy();
                        Ret = false;
                    }
                    else 
                    {
                        printf("Loaded texture '%s'\n", FullPath.c_str());
                    }
                }
            }
        }

        _sampler.Create();
        _sampler.Parameter(Eugenix::Render::TextureParam::MinFilter, Eugenix::Render::TextureFilter::Linear);
        _sampler.Parameter(Eugenix::Render::TextureParam::MagFilter, Eugenix::Render::TextureFilter::Linear);
        _sampler.Parameter(Eugenix::Render::TextureParam::WrapS, Eugenix::Render::TextureWrapping::ClampToEdge);
        _sampler.Parameter(Eugenix::Render::TextureParam::WrapT, Eugenix::Render::TextureWrapping::ClampToEdge);

        return Ret;
    }

    void PopulateBuffers()
    {
        _buffers[POS_VB].Storage(Eugenix::Core::MakeData(m_Positions));
        _buffers[TEXCOORD_VB].Storage(Eugenix::Core::MakeData(m_TexCoords));
        _buffers[NORMAL_VB].Storage(Eugenix::Core::MakeData(m_Normals));
        _buffers[INDEX_BUFFER].Storage(Eugenix::Core::MakeData(m_Indices));

        _vao.AttachVertices(0, _buffers[POS_VB], sizeof(m_Positions[0]));
        _vao.AttachVertices(1, _buffers[TEXCOORD_VB], sizeof(m_TexCoords[0]));
        _vao.AttachVertices(2, _buffers[NORMAL_VB], sizeof(m_Normals[0]));
        _vao.AttachIndices(_buffers[INDEX_BUFFER]);

        _vao.Attribute({ POSITION_LOCATION,  3, Eugenix::Render::DataType::Float, false, 0, 0 });
        _vao.Attribute({ TEX_COORD_LOCATION, 2, Eugenix::Render::DataType::Float, false, 0, 1 });
        _vao.Attribute({ NORMAL_LOCATION,    3, Eugenix::Render::DataType::Float, false, 0, 2 });
    }

#define INVALID_MATERIAL 0xFFFFFFFF


    enum BUFFER_TYPE {
        INDEX_BUFFER = 0,
        POS_VB = 1,
        TEXCOORD_VB = 2,
        NORMAL_VB = 3,
        WVP_MAT_VB = 4,
        WORLD_MAT_VB = 5,
        NUM_BUFFERS = 6
    };

    WorldTransform m_worldTransform;

    Eugenix::Render::OpenGL::VertexArray _vao;
    Eugenix::Render::OpenGL::Buffer _buffers[NUM_BUFFERS] = {};

    //GLuint m_VAO = 0;
    //GLuint m_Buffers[NUM_BUFFERS] = { 0 };

    struct BasicMeshEntry {
        BasicMeshEntry()
        {
            NumIndices = 0;
            BaseVertex = 0;
            BaseIndex = 0;
            MaterialIndex = INVALID_MATERIAL;
        }

        unsigned int NumIndices;
        unsigned int BaseVertex;
        unsigned int BaseIndex;
        unsigned int MaterialIndex;
    };

    std::vector<BasicMeshEntry> m_Meshes;
    std::vector<Eugenix::Render::OpenGL::Texture2D> m_Textures;
    Eugenix::Render::OpenGL::Sampler _sampler;

    // Temporary space for vertex stuff before we load them into the GPU
    std::vector<glm::vec3> m_Positions;
    std::vector<glm::vec3> m_Normals;
    std::vector<glm::vec2> m_TexCoords;
    std::vector<unsigned int> m_Indices;

    // «десь ему не место?..
    Eugenix::Assets::ImageLoader _imageLoader;
};


#endif  /* OGLDEV_BASIC_MESH_H */