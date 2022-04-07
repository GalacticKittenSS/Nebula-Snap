project "Snap"
	kind "ConsoleApp"
	
	cppdialect "C++17"
	language "C++"

	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-obj/" .. outputdir .. "/%{prj.name}")

	files {
		"src/**.h",
		"src/**.cpp"
	}

	includedirs {
		"%{wks.location}/Nebula/include",
		"%{includedir.Entt}",
		"%{includedir.ImGui}",
		"%{includedir.Spdlog}"
	}

	libdirs { "%{wks.location}/Nebula/Modules" }

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "NB_DEBUG"
		runtime "Debug"
		symbols "on"

		postbuildcommands {
			"{COPYDIR} \"%{LibraryDir.VulkanSDK_DebugDLL}\" \"%{cfg.targetdir}\""
		}

		links {
			"Nebula_Debug.lib",
			"GLFW_Debug.lib",
			"GLad_Debug.lib"
		}	

	filter "configurations:Release"
		defines "NB_DEBUG"
		runtime "Release"
		optimize "on"

		links {
			"Nebula.lib",
			"GLFW.lib",
			"GLad.lib"
		}	

	filter "configurations:Dist"
		defines "NB_DIST"
		runtime "Release"
		optimize "on"

		links {
			"Nebula.lib",
			"GLFW.lib",
			"GLad.lib"
		}	