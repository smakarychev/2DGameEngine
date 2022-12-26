require "export-compile-commands/export-compile-commands"
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
	include "Engine/vendor/imgui"	
	include "Engine/vendor/msdf-atlas-gen"
	include "Engine/vendor/yaml-cpp"
	
group""
	include "Engine"
	include "Sandbox"