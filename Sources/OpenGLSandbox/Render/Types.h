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

	enum struct PipelineFeature
	{
		DepthTest,
		CullFace,
		Blend,
		ScissorTest,
		Multisample,
		StencilTest,
		PolygonOffsetFill
	};

	enum struct ClearFlags : uint32_t
	{
		Color = 1 << 0, 
		Depth = 1 << 1, 
		Stencil = 1 << 2
	};

	constexpr ClearFlags operator|(ClearFlags lhs, ClearFlags rhs)
	{
		return static_cast<ClearFlags>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
	}

	// TODO : rename to binding (see SharedData)
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
