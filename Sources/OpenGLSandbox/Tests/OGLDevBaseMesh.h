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
        // Release the previously loaded mesh (if it exists)
        Clear();

        // Create the VAO
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        // Create the buffers for the vertices attributes
        glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

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
        glBindVertexArray(m_VAO);

        for (unsigned int i = 0; i < m_Meshes.size(); i++) {
            unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;

            assert(MaterialIndex < m_Textures.size());

            if (m_Textures[MaterialIndex]) 
            {
                m_Textures[MaterialIndex]->Bind(COLOR_TEXTURE_UNIT);
            }

            glDrawElementsBaseVertex(GL_TRIANGLES,
                m_Meshes[i].NumIndices,
                GL_UNSIGNED_INT,
                (void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
                m_Meshes[i].BaseVertex);
        }

        // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);
    }

    void Render(unsigned int NumInstances, const glm::mat4* WVPMats, const glm::mat4* WorldMats)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WVP_MAT_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * NumInstances, WVPMats, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WORLD_MAT_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * NumInstances, WorldMats, GL_DYNAMIC_DRAW);

        glBindVertexArray(m_VAO);

        for (unsigned int i = 0; i < m_Meshes.size(); i++) 
        {
            const unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;

            assert(MaterialIndex < m_Textures.size());

            if (m_Textures[MaterialIndex]) 
            {
                m_Textures[MaterialIndex]->Bind(GL_TEXTURE0);
            }

            glDrawElementsInstancedBaseVertex(GL_TRIANGLES,
                m_Meshes[i].NumIndices,
                GL_UNSIGNED_INT,
                (void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
                NumInstances,
                m_Meshes[i].BaseVertex);
        }

        // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);
    }

    WorldTransform& GetWorldTransform() { return m_worldTransform; }

private:
    void Clear()
    {
        for (unsigned int i = 0; i < m_Textures.size(); i++) 
        {
            SAFE_DELETE(m_Textures[i]);
        }

        if (m_Buffers[0] != 0) 
        {
            glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
        }

        if (m_VAO != 0) 
        {
            glDeleteVertexArrays(1, &m_VAO);
            m_VAO = 0;
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
        for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {
            const aiMaterial* pMaterial = pScene->mMaterials[i];

            m_Textures[i] = NULL;

            if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString Path;

                if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) 
                {
                    std::string p(Path.data);

                    if (p.substr(0, 2) == ".\\") 
                    {
                        p = p.substr(2, p.size() - 2);
                    }

                    std::string FullPath = Dir + "/" + p;

                    /*m_Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

                    if (!m_Textures[i]->Load()) {
                        printf("Error loading texture '%s'\n", FullPath.c_str());
                        delete m_Textures[i];
                        m_Textures[i] = NULL;
                        Ret = false;
                    }
                    else {
                        printf("Loaded texture '%s'\n", FullPath.c_str());
                    }*/
                }
            }
        }

        return Ret;
    }

    void PopulateBuffers()
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_Positions[0]) * m_Positions.size(), &m_Positions[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(POSITION_LOCATION);
        glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_TexCoords[0]) * m_TexCoords.size(), &m_TexCoords[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(TEX_COORD_LOCATION);
        glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_Normals[0]) * m_Normals.size(), &m_Normals[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(NORMAL_LOCATION);
        glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);
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
    GLuint m_VAO = 0;
    GLuint m_Buffers[NUM_BUFFERS] = { 0 };

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
    std::vector<Eugenix::Render::OpenGL::Texture2D*> m_Textures;

    // Temporary space for vertex stuff before we load them into the GPU
    std::vector<glm::vec3> m_Positions;
    std::vector<glm::vec3> m_Normals;
    std::vector<glm::vec2> m_TexCoords;
    std::vector<unsigned int> m_Indices;
};


#endif  /* OGLDEV_BASIC_MESH_H */