#pragma once

#include <cstdint>

namespace Eugenix
{
	namespace Render::OpenGL
	{
		class Object
		{
		public:
			virtual void Create() = 0;
			virtual void Destroy() = 0;

			[[nodiscard, maybe_unused]] uint32_t Handle() const { return _handle; }
			[[nodiscard, maybe_unused]] uint32_t Extra() const { return _extra; }

		protected:
			uint32_t _handle{};
			uint32_t _extra{};
		};
	} // namespace OpenGL
} // namespace Eugenix
