include "dependencies.lua"


workspace "2DGameEngine"
	configurations { "Debug", "Release", "Dist"}
	architecture "x86_64"
	startproject "Sandbox"
	
	flags
	{
		"MultiProcessorCompile"
	}

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "Engine/vendor/GLFW"
	include "Engine/vendor/glad"


group""

	include "Engine"
	include "Sandbox"