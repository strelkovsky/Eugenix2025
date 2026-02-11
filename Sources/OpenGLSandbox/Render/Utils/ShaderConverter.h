#pragma once

#include <filesystem>

namespace fs = std::filesystem;

namespace Eugenix
{
	class ShaderConverter
	{
	public:
		ShaderConverter() = delete;

		static void Convert(const std::filesystem::path& inputDir, const std::filesystem::path& outputDir)
		{
			fs::create_directories(outputDir);

			for (const fs::directory_entry& entry : fs::directory_iterator(inputDir))
			{
				if (entry.is_regular_file())
				{
					ConvertFile(entry.path(), outputDir);
				}
			}
		}

		static void ConvertFile(const std::filesystem::path& file, const std::filesystem::path& outputDir)
		{
			auto stage = file.extension().string();
			if (!stage.empty() && stage.front() == '.')
				stage.erase(stage.begin());

			if (stage.empty())
				throw std::runtime_error("Shader file has no stage extension");

			fs::path outFile = outputDir / file.filename();
			outFile += ".spv";

			const auto cmd = std::format(
				"glslangvalidator -G -S {} -o \"{}\" \"{}\"",
				stage, 
				outFile.string(),
				file.string());

			 const auto rc = std::system(cmd.c_str());
			 if (rc != 0)
			     throw std::runtime_error(std::format("glslangvalidator failed (rc={}): {}", rc, cmd));
		}
	};
} // namespace Eugenix