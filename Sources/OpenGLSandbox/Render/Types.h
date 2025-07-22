#pragma once

namespace Eugenix
{
	namespace Render
	{
		enum struct DataType
		{
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
	} // namespace Render
} // namespace Eugenix
