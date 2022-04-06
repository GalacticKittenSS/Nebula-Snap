--include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Snap"
	architecture "x86_64"
	startproject "Snap"

	configurations {
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--Client
include "Snap"