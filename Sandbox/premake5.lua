project "Sandbox"
	kind "ConsoleApp"	
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

		"%{wks.location}/Engine/src",
		"%{wks.location}/Engine/vendor/**/include",
	}

	links
	{
		"Engine",
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