project "Engine"
	kind "StaticLib"	
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "enginepch.h"
	pchsource "src/enginepch.cpp"

	files
	{
		"src/**.cpp",
		"src/**.h"
	}

	includedirs 
	{
		"src/",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.msdf_atlas}",
		"%{IncludeDir.msdfgen}",	
		"%{IncludeDir.imgui}",	
	}

	links
	{
		"GLFW",
		"glad",
		"msdfgen",
		"msdf-atlas-gen",
		"imgui",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"STB_IMAGE_IMPLEMENTATION",
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