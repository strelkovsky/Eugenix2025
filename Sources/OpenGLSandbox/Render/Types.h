#pragma once

namespace Eugenix
{
	namespace Render
	{
		enum struct DataType
		{
			UByte,
			UInt,
			Float
		};

		enum struct PrimitiveType
		{
			Triangles
		};

		enum struct ShaderStageType
		{
			Vertex,
			Fragment,
			Compute
		};

		enum struct BufferTarget
		{
			Uniform
		};

		// TODO : rename to SamplerParam when DSA?..
		enum struct TextureParam
		{
			WrapS,
			WrapT,
			MinFilter,
			MagFilter
		};

		enum struct TextureWrapping
		{
			Repeat,
			MirroredRepeat,
			ClampToEdge,
			ClampToBorder,
		};

		// TODO : check naming
		enum struct TextureFilter
		{
			Linear,
		};
	} // namespace Render
} // namespace Eugenix
