#pragma once

namespace Eugenix::Render
{
	enum struct DataType
	{
		UByte,
		UInt,
		Float
	};

	enum struct PrimitiveType
	{
		Triangles,
		Lines
	};

	enum struct ShaderStageType
	{
		Vertex,
		Fragment,
		Compute
	};

	enum struct BufferTarget
	{
		UBO
	};

	// TODO : rename to SamplerParam when DSA?..
	enum struct TextureParam
	{
		WrapS,
		WrapT,
		WrapR,
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
		Nearest
	};

	enum struct TextureColorSpace 
	{ 
		Linear, 
		SRGB 
	};

	// TODO : rename to binding
	enum struct BufferBinding
	{
		/* vertex */

		Transform,
		Camera,

		/* fragment */

		Material,
		Lighting,
	};
} // namespace Eugenix::Render
