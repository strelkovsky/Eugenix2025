// STD headers
#include <iostream>

// ThirdParty headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Engine headers
#include "Engine/IO/IO.h"

// Sandbox headers
#include "App/SandboxApp.h"

class SquareApp final : public Eugenix::SandboxApp
{
protected:
	bool OnInit() override
	{ 
		const std::vector<float> square_vertices
		{
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f
		};

		const std::vector<uint32_t> square_elements
		{
			0, 1, 2, 
			1, 2, 3
		};

		glCreateVertexArrays(1, &_squareVao);

		glCreateBuffers(1, &_squareVbo);
		glNamedBufferData(_squareVbo, square_vertices.size() * sizeof(square_vertices), square_vertices.data(), GL_STATIC_DRAW);

		glCreateBuffers(1, &_squareEbo);
		glNamedBufferData(_squareEbo, square_elements.size() * sizeof(uint32_t), square_elements.data(), GL_STATIC_DRAW);

		glVertexArrayVertexBuffer(_squareVao, 0, _squareVbo, 0, sizeof(float) * 3);
		glVertexArrayElementBuffer(_squareVao, _squareEbo);

		glVertexArrayAttribFormat(_squareVao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(_squareVao, 0, 0);
		glEnableVertexArrayAttrib(_squareVao, 0);

		const auto vsSourceData = Eugenix::IO::FileContent("Shaders/simple.vert");
		const char* vsSource = vsSourceData.data();

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vsSource, nullptr);
		glCompileShader(vertexShader);

		const auto fsSourceData = Eugenix::IO::FileContent("Shaders/simple.frag");
		const char* fsSource = fsSourceData.data();

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fsSource, nullptr);
		glCompileShader(fragmentShader);

		_squarePipeline = glCreateProgram();
		glAttachShader(_squarePipeline, vertexShader);
		glAttachShader(_squarePipeline, fragmentShader);
		glLinkProgram(_squarePipeline);
		glUseProgram(_squarePipeline);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		return true; 
	}

	void OnRender() override
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(_squarePipeline);

		glBindVertexArray(_squareVao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}
	
	void OnCleanup() override
	{
		glDeleteProgram(_squarePipeline);
		glDeleteVertexArrays(1, &_squareVao);
		glDeleteBuffers(1, &_squareVbo);
	}

private:
	uint32_t _squareVao{};
	uint32_t _squareVbo{};
	uint32_t _squareEbo{};

	uint32_t _squarePipeline{};
};

int main()
{
	SquareApp app;
	return app.Run();
}