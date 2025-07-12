#pragma once

#include "Render/OpenGL/ShaderStage.h"

// TMP bridge beetween tests & sandbox sources
namespace Eugenix
{
	Eugenix::Render::OpenGL::ShaderStage CreateStage(const char* source, Eugenix::Render::ShaderStageType type)
	{
		Eugenix::Render::OpenGL::ShaderStage stage{ type };
		stage.Create();
		stage.CompileFromSource(source);
		return stage;
	}
}
