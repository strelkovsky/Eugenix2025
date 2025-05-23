// STD headers
#include <iostream>

// ThirdParty headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Engine headers
#include "Engine/IO/IO.h"

// Sandbox headers
#include "App/SandboxApp.h"

class TriangleApp final : public Eugenix::SandboxApp
{
protected:
	bool OnInit() override
	{ 
		const float vertices[] =
		{
			-0.5f, -0.5f, 0.0f,
			 0.0f,  0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f
		};

		glGenVertexArrays(1, &_triangleVao);
		glBindVertexArray(_triangleVao);

		glGenBuffers(1, &_triangleVbo);
		glBindBuffer(GL_ARRAY_BUFFER, _triangleVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

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

		_trianglePipeline = glCreateProgram();
		glAttachShader(_trianglePipeline, vertexShader);
		glAttachShader(_trianglePipeline, fragmentShader);
		glLinkProgram(_trianglePipeline);
		glUseProgram(_trianglePipeline);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		return true; 
	}

	void OnRender() override
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(_trianglePipeline);

		glBindVertexArray(_triangleVao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	
	void OnCleanup() override
	{
		glDeleteProgram(_trianglePipeline);
		glDeleteVertexArrays(1, &_triangleVao);
		glDeleteBuffers(1, &_triangleVbo);
	}

private:
	uint32_t _triangleVao{};
	uint32_t _triangleVbo{};

	uint32_t _trianglePipeline{};
};

int main()
{
	TriangleApp app;
	return app.Run();
}