#pragma once

#include <cstdint>

#include <btBulletCollisionCommon.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

// Engine headers
#include "Engine/IO/IO.h"

// Sandbox headers
#include "App/SandboxApp.h"
#include "Render/OpenGL/Buffer.h"
#include "Render/OpenGL/Commands.h"
#include "Render/Mesh.h"
#include "Render/OpenGL/Pipeline.h"
#include "Render/OpenGL/VertexArray.h"
#include "Render/Utils/MeshGenerator.h"

#include "../Tests/TestUtils.h"

namespace core::data
{
    struct GridPosition
    {
        int32_t row{ };
        int32_t col{ };

        bool operator==(const GridPosition& position) const
        {
            return row == position.row && col == position.col;
        }
    };

    template <class T, int32_t R, int32_t C>
    class Grid
    {
    public:
        using ValueType = T;
        static constexpr int Rows = R;
        static constexpr int Cols = C;

        T& At(const GridPosition& p) { return _data[IndexAt(p)]; }
        const T& At(const GridPosition& p) const { return _data[IndexAt(p)]; }

        // NOTE : lower-case for stl compat
        auto begin() -> auto { return _data.begin(); }
        auto end() -> auto { return _data.end(); }
        auto begin() const { return _data.begin(); }
        auto end()   const { return _data.end(); }

        static constexpr bool WithinBounds(const GridPosition& p)
        {
            return (p.row >= 0 && p.row < R) && (p.col >= 0 && p.col < C);
        }

    private:
        static constexpr size_t IndexAt(GridPosition p)
        {
            assert(WithinBounds(p));
            return size_t(p.row) * C + size_t(p.col);
        }

    protected:
        std::array<T, size_t(R) * size_t(C)> _data{};
    };
}

std::vector<float> debug_vertices;

class PhysicsDebug : public btIDebugDraw
{
public:
    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
    {
        debug_vertices.push_back(from.x());
        debug_vertices.push_back(from.y());
        debug_vertices.push_back(from.z());

        debug_vertices.push_back(to.x());
        debug_vertices.push_back(to.y());
        debug_vertices.push_back(to.z());
    }

    void clearLines() override
    {
        debug_vertices.clear();
    }

    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override {}
    void reportErrorWarning(const char* warningString) override {}
    void draw3dText(const btVector3& location, const char* textString) override {}
    void setDebugMode(int debugMode) override { _mode = debugMode; }
    int getDebugMode() const override { return _mode; }

private:
    int _mode = btIDebugDraw::DBG_DrawWireframe;
};

namespace game
{
    enum class piece_type
    {
        none,
        x,
        o
    };

    struct piece
    {
        glm::vec3 position;
        piece_type type;

        // TODO move the position a little closer to the center
        // TODO make the winner row/column white? and maybe blinking
    };

    class Board final : public core::data::Grid<piece, 3, 3>
    {
    public:
        auto Reset() -> void
        {
            for (auto& cell : _data)
                cell.type = piece_type::none;
        }

        auto CheckWin(const core::data::GridPosition& position, piece_type type) const -> bool
        {
            return checkRow(position.row, type) || checkCol(position.col, type) || checkDiagonals(type);
        }

    private:
        auto checkRow(int32_t row, piece_type type) const -> bool
        {
            return At({ row, 0 }).type == type &&
                At({ row, 1 }).type == type &&
                At({ row, 2 }).type == type;
        }
        auto checkCol(int32_t column, piece_type type) const -> bool
        {
            return At({ 0, column }).type == type &&
                At({ 1, column }).type == type &&
                At({ 2, column }).type == type;
        }
        auto checkDiagonals(piece_type type) const -> bool
        {
            return At({ 1, 1 }).type == type &&
                ((At({ 0, 0 }).type == type && At({ 2, 2 }).type == type) || (At({ 0, 2 }).type == type && At({ 2, 0 }).type == type));
        }
    };
}

auto is_editor = false;

static game::Board board;

auto x_turn = true;
auto is_end = false;

static std::unique_ptr<btCollisionWorld> world2;

static auto cursor_x = 0.0f;
static auto cursor_y = 0.0f;

namespace core::data
{
    struct camera
    {
        glm::mat4 view;
        glm::mat4 proj;
    };
}

namespace core::data
{
    template <typename vertex, typename primitive>
    struct geometry
    {
        std::vector<vertex>    vertices;
        std::vector<primitive> elements;
    };
}

namespace core::primitive
{
    struct triangle
    {
        uint32_t a;
        uint32_t b;
        uint32_t c;

        static constexpr uint32_t elements{ 3 };
    };
}

static core::data::camera camera_data
{
    glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)),
    glm::perspective(glm::radians(60.0f), static_cast<float>(1024) / static_cast<float>(768), 0.1f, 100.0f)
};

namespace Eugenix
{
    class TicTacToeApp final : public SandboxApp
    {
    protected:
        bool onInit() override
        {
            _pipeline = Eugenix::MakePipelineFromFiles("Shaders/simple_pos_transform_ubo.vert", "Shaders/simple_pos_transform_ubo.frag");

            constexpr Eugenix::Render::Attribute position_attribute{ 0, 3, Eugenix::Render::DataType::Float, GL_FALSE,  0 };

            Assimp::Importer pieces_importer;

            const auto pieces = pieces_importer.ReadFile("Models/tictactoe/grid_pieces.obj", 0);
            const auto o_mesh = pieces->mMeshes[1];
            const auto x_mesh = pieces->mMeshes[0];

            for (auto i = 0; i < o_mesh->mNumVertices; i++)
            {
                const auto& vertex = o_mesh->mVertices[i];
                o_geometry.vertices.emplace_back(vertex.x, vertex.y, vertex.z);
            }

            for (auto i = 0; i < o_mesh->mNumFaces; i++)
            {
                const auto& face = o_mesh->mFaces[i];
                o_geometry.elements.emplace_back(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
            }

            oVbo.Create();
            oVbo.Storage(Core::MakeData(o_geometry.vertices));

            oEbo.Create();
            oEbo.Storage(Core::MakeData(o_geometry.elements));

            _oVao.Create();
            _oVao.AttachVertices(oVbo, sizeof(glm::vec3));
            _oVao.AttachIndices(oEbo);
            _oVao.Attribute(position_attribute);

            for (auto i = 0; i < x_mesh->mNumVertices; i++)
            {
                const auto& vertex = x_mesh->mVertices[i];
                x_geometry.vertices.emplace_back(vertex.x, vertex.y, vertex.z);
            }

            for (auto i = 0; i < x_mesh->mNumFaces; i++)
            {
                const auto& face = x_mesh->mFaces[i];
                x_geometry.elements.emplace_back(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
            }

            xVbo.Create();
            xVbo.Storage(Core::MakeData(x_geometry.vertices));

            xEbo.Create();
            xEbo.Storage(Core::MakeData(x_geometry.elements));

            _xVao.Create();
            _xVao.AttachVertices(xVbo, sizeof(glm::vec3));
            _xVao.AttachIndices(xEbo);
            _xVao.Attribute(position_attribute);

            Assimp::Importer grid_importer;

            const auto grid_scene = grid_importer.ReadFile("Models/tictactoe/grid.obj", 0);
            const auto grid_mesh = grid_scene->mMeshes[0];

            for (auto i = 0; i < grid_mesh->mNumVertices; i++)
            {
                const auto& vertex = grid_mesh->mVertices[i];
                grid_geometry.vertices.emplace_back(vertex.x, vertex.y, vertex.z);
            }

            for (auto i = 0; i < grid_mesh->mNumFaces; i++)
            {
                const auto& face = grid_mesh->mFaces[i];
                grid_geometry.elements.emplace_back(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
            }

            gridVbo.Create();
            gridVbo.Storage(Core::MakeData(grid_geometry.vertices));

            gridEbo.Create();
            gridEbo.Storage(Core::MakeData(grid_geometry.elements));

            _gridVao.Create();
            _gridVao.AttachVertices(gridVbo, sizeof(glm::vec3));
            _gridVao.AttachIndices(gridEbo);
            _gridVao.Attribute(position_attribute);

            //camera_data.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f));
            //camera_data.proj = glm::perspective(glm::radians(60.0f), static_cast<float>(width()) / static_cast<float>(height()), 0.1f, 100.0f);

            _transformUbo.Create();
            _transformUbo.Storage(Core::MakeData(&_model), GL_DYNAMIC_STORAGE_BIT);
            glBindBufferBase(GL_UNIFORM_BUFFER, (GLint)UBO::Location::Transform, _transformUbo.NativeHandle());
            //transform_ubo.bind_base(opengl::constants::uniform_buffer, core::buffer::transform);

            _cameraUbo.Create();
            _cameraUbo.Storage(Core::MakeData(&camera_data), GL_DYNAMIC_STORAGE_BIT);
            glBindBufferBase(GL_UNIFORM_BUFFER, (GLint)UBO::Location::Camera, _cameraUbo.NativeHandle());
            //_cameraUbo.bind_base(opengl::constants::uniform_buffer, core::buffer::camera);

            _materialUbo.Create();
            _materialUbo.Storage(Core::MakeData(&_materialAlbedo), GL_DYNAMIC_STORAGE_BIT);
            glBindBufferBase(GL_UNIFORM_BUFFER, (GLint)UBO::Location::Material, _materialUbo.NativeHandle());
            //_materialUbo.bind_base(opengl::constants::uniform_buffer, core::buffer::material);

            auto bt_default_configuration = new btDefaultCollisionConfiguration();
            world2 = std::make_unique<btCollisionWorld>(
                new btCollisionDispatcher(bt_default_configuration), 
                new btDbvtBroadphase(), 
                bt_default_configuration);

            _physDebug = std::make_unique<PhysicsDebug>();
            _physDebug->setDebugMode(
                btIDebugDraw::DBG_DrawWireframe |
                btIDebugDraw::DBG_DrawAabb
            );
            world2->setDebugDrawer(_physDebug.get());

            _tileShape = std::make_unique<btBoxShape>(btVector3(0.5f, 0.5f, 0.105f));

            for (auto row = 0; row < board.Rows; row++)
            {
                constexpr auto       piece_size = 1.5f;
                const     auto  x = -piece_size + row * piece_size;

                for (auto col = 0; col < board.Cols; col++)
                {
                    const auto y = piece_size - col * piece_size;

                    board.At({ row, col }).position = { x, y, 0.0f };

                    printf("create shape {%f} {%f}\n", x, y);

                    btTransform transform;
                    transform.setIdentity();
                    transform.setOrigin(btVector3(x, y, 0.0f));

                    auto bt_collision_object = new btCollisionObject();
                    bt_collision_object->setCollisionShape(_tileShape.get());
                    bt_collision_object->setWorldTransform(transform);
                    bt_collision_object->setUserIndex(row);
                    bt_collision_object->setUserIndex2(col);

                    world2->addCollisionObject(bt_collision_object);
                }
            }

            debugVbo.Create();
            const auto debugCapacityBytes = 1024 * 1024; // 1 MB, например
            debugVbo.Storage(Core::Data{ nullptr, debugCapacityBytes }, GL_DYNAMIC_STORAGE_BIT);

            _debugVao.Create();
            _debugVao.AttachVertices(debugVbo, sizeof(glm::vec3));
            _debugVao.Attribute(position_attribute);

            Render::OpenGL::Commands::Clear(0.42745098039215684f, 0.8823529411764706f, 0.8235294117647058f);

            return true;
        }

        virtual void onKeyHandle(int key, int code, int action, int mode) override
        {
            if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
            {
                board.Reset();

                x_turn = true;
                is_end = false;
            }

            if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
            {
                is_editor = !is_editor;
            }
        }

        void onMouseHandle(double xPos, double yPos) override
        {
            cursor_x = xPos;
            cursor_y = yPos;
        }

        void onMouseButtonHandle(int button, int action, int mods) override
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !is_end)
            {
                const glm::vec4 viewport{ 0.0f, 0.0f, (float)width(), (float)height() };

                auto start = glm::unProject(glm::vec3(cursor_x, height() - cursor_y, -1.0f), camera_data.view, camera_data.proj, viewport);
                auto end = glm::unProject(glm::vec3(cursor_x, height() - cursor_y, 1.0f), camera_data.view, camera_data.proj, viewport);
                end = start + glm::normalize(end - start) * 1000.0f;

                const btVector3 from(start.x, start.y, start.z);
                const btVector3   to(end.x, end.y, end.z);

                btCollisionWorld::ClosestRayResultCallback result(from, to);
                world2->rayTest(from, to, result);
                if (result.hasHit())
                {
                    core::data::GridPosition position
                    {
                        result.m_collisionObject->getUserIndex(),
                        result.m_collisionObject->getUserIndex2()
                    };

                    printf("has hit - {%d}\n", board.At(position).type);

                    if (board.At(position).type == game::piece_type::none)
                    {
                        if (x_turn) // TODO make some piece type variable and use the code just once
                        {
                            board.At(position).type = game::piece_type::x;
                            is_end = board.CheckWin(position, game::piece_type::x);
                        }
                        else
                        {
                            board.At(position).type = game::piece_type::o;
                            is_end = board.CheckWin(position, game::piece_type::o);
                        }

                        printf("is end - %d\n", (is_end) ? 1 : 0);

                        x_turn = !x_turn;
                    }
                }
            }
        }

        void onRender() override
        {
            Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            _pipeline.Bind();

            _model = glm::mat4(1.0f);
            _transformUbo.Update(Core::MakeData(&_model));

            if (is_editor)
            {
                _materialUbo.Update(Core::MakeData(&grid_color));

                _physDebug->clearLines();
                world2->debugDrawWorld();

                debugVbo.Update(Core::MakeData(debug_vertices));

                _debugVao.Bind();
                Render::OpenGL::Commands::DrawVertices(Render::PrimitiveType::Lines, debug_vertices.size() / 3);
            }
            else
            {
                Render::OpenGL::Commands::Clear(0.42745098039215684f, 0.8823529411764706f, 0.8235294117647058f);
                Render::OpenGL::Commands::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }

            _materialAlbedo = glm::vec3(0.0f, 1.0f, 0.0f);
            _materialUbo.Update(Core::MakeData(&grid_color));

            _gridVao.Bind();
            Render::OpenGL::Commands::DrawIndexed(Render::PrimitiveType::Triangles, grid_geometry.elements.size() * core::primitive::triangle::elements, Render::DataType::UInt);

            for (auto& [piece_position, piece_type] : board)
            {
                _model = glm::translate(glm::mat4(1.0f), piece_position);

                _transformUbo.Update(Core::MakeData(&_model));

                switch (piece_type)
                {
                case game::piece_type::x:
                {
                    _xVao.Bind();
                    _materialUbo.Update(Core::MakeData(&x_color));
                    Render::OpenGL::Commands::DrawIndexed(Render::PrimitiveType::Triangles, x_geometry.elements.size() * core::primitive::triangle::elements, Render::DataType::UInt);

                    break;
                }
                case game::piece_type::o:
                {
                    _oVao.Bind();
                    _materialUbo.Update(Core::MakeData(&o_color));
                    Render::OpenGL::Commands::DrawIndexed(Render::PrimitiveType::Triangles, o_geometry.elements.size() * core::primitive::triangle::elements, Render::DataType::UInt);

                    break;
                }
                default:
                    break;
                }
            }
        }

    private:
        //core::data::camera camera_data;

        Render::OpenGL::Pipeline _pipeline;

        Render::OpenGL::Buffer oVbo;
        Render::OpenGL::Buffer oEbo;
        Render::OpenGL::Buffer xVbo;
        Render::OpenGL::Buffer xEbo;
        Render::OpenGL::Buffer gridVbo;
        Render::OpenGL::Buffer gridEbo;
        Render::OpenGL::Buffer debugVbo;

        Render::OpenGL::VertexArray _oVao{};
        Render::OpenGL::VertexArray _xVao{};
        Render::OpenGL::VertexArray _gridVao{};
        Render::OpenGL::VertexArray _debugVao{};

        core::data::geometry<glm::vec3, core::primitive::triangle>    o_geometry;
        core::data::geometry<glm::vec3, core::primitive::triangle>    x_geometry;
        core::data::geometry<glm::vec3, core::primitive::triangle> grid_geometry;

        // UBOs
        Render::OpenGL::Buffer _transformUbo;
        Render::OpenGL::Buffer _cameraUbo;
        Render::OpenGL::Buffer _materialUbo;

        // UBO data
        glm::mat4 _model{ 1.0f };
        glm::vec3 _materialAlbedo{ 1.0f };
        glm::vec3 x_color{ 1.0f, 0.8392156862745098f, 0.22745098039215686f };
        glm::vec3 o_color{ 0.9686274509803922f, 0.35294117647058826f, 0.35294117647058826f };
        glm::vec3 grid_color{ 1.0f, 0.6627450980392157f, 0.3333333333333333f };

        //std::unique_ptr<btCollisionWorld> _world;
        std::unique_ptr<PhysicsDebug> _physDebug;
        std::unique_ptr<btCollisionShape> _tileShape;
    };
} // namespace Eugenix