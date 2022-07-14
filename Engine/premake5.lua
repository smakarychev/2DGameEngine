project "Engine"
	kind "StaticLib"	
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.cpp",
		"src/**.h"
	}

	includedirs 
	{
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glad}",
	}

	links
	{
		"GLFW",
		"glad"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	filter "system:windows"
		systemversion "latest"

	filter { "configurations:Debug" }
		defines "ENGINE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter { "configurations:Release" }
		defines "ENGINE_RELEASE"
		runtime "Release"
		optimize "on"

	filter { "configurations:Dist" }
		defines "ENGINE_DIST"
		runtime "Release"
		optimize "speed"