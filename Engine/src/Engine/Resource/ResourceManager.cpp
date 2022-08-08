#include "enginepch.h"

#include "ResourceManager.h"

#include "Engine/Core/Log.h"

namespace Engine
{
	std::string ResourceManager::ReadFile(std::filesystem::path path)
	{
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (in)
		{
			return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
		}
		ENGINE_CORE_ERROR("ResourceManager: error reading file {}", path.string());
		return "";
	}
}