#pragma once

#include "Engine/Core/Types.h"

#include <filesystem>

namespace Engine
{
	using namespace Types;

	class ResourceManager
	{
		static std::string ReadFile(std::filesystem::path path);
	};
}