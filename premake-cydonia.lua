workspace "Cydonia"
	location "build"
	configurations { "Debug", "Release", "Profiling" }
	startproject "Sandbox"

	objdir "intermediate"
	targetdir "bin/%{cfg.buildcfg}"
	debugdir "bin/%{cfg.buildcfg}"

	filter "configurations:Debug"
		defines { "CYD_DEBUG",
				  "CYD_ASSERTIONS_ENABLED",
				  "CYD_GPU_PROFILING" }
		symbols "On"

	filter "configurations:Profiling"
		defines { "NDEBUG",
				  "TRACY_ENABLE",
				  "CYD_ASSERTIONS_ENABLED",
				  "CYD_PROFILING",
				  "CYD_GPU_PROFILING" }
		optimize "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

project "Emporium"
	location "Build/Emporium"
	language "C++"
	cppdialect "C++20"
	kind "StaticLib"
	architecture "x86_64"

	includedirs { "Emporium" }

	files { "Emporium/**.h", "Emporium/**.cpp" }

project "Engine"
	location "Build/Engine"
	language "C++"
	cppdialect "C++20"
	kind "StaticLib"
	architecture "x86_64"
	linkoptions { "-IGNORE:4006" }

	-- Linking dependencies and include
	includedirs { "Engine",
				  "Emporium",
				  "include",
				  "$(VULKAN_SDK)/include" }

	libdirs { "lib/%{cfg.architecture}" }
	
	links { "Emporium",
			"glfw3",
			"$(VULKAN_SDK)/lib/vulkan-1.lib",
			"d3d12",
			"dxgi",
			"d3dcompiler" }

	-- Adding all file types, including shaders as we custom build them
	files { "Engine/*.h",
			"Engine/*.cpp",
			"Engine/ECS/**.h",
			"Engine/ECS/**.cpp",
			"Engine/Graphics/**.h",
			"Engine/Graphics/**.cpp",
			"Engine/Physics/**.h",
			"Engine/Physics/**.cpp",
			"Engine/UI/**.h",
			"Engine/UI/**.cpp",
			"Engine/Input/**.h",
			"Engine/Input/**.cpp",
			"Engine/ThirdParty/ImGui/**.h",
			"Engine/ThirdParty/ImGui/**.cpp",
			"Engine/ThirdParty/Tracy/TracyClient.cpp",
			"Engine/Data/*.json" }

	removefiles { "Engine/Input/SDLWindow.h",
				  "Engine/Input/SDLWindow.cpp" }

	filter "files:Engine/Data/*.json"
		buildmessage "Copying JSON Data %{file.abspath}"

		buildcommands {
			"{COPY} %{file.abspath} %{cfg.targetdir}"
		}

		buildoutputs { "%{cfg.targetdir}/%{file.basename}.json" }

-- ===============================================================================================
-- Applications
project "Sandbox"
	location "Build/Sandbox"
	language "C++"
	cppdialect "C++20"
	kind "ConsoleApp"
	architecture "x86_64"

	includedirs { "Sandbox", "Engine", "Emporium", "include" }
	links { "Engine" }

	files { "Sandbox/**.h",
			"Sandbox/**.cpp",
			"CydoniaIcon.png" }

	filter {}

	filter { 'system:windows' }
		files { 'Sandbox/Sandbox.rc', '**.ico' }

	filter {}

	filter "files:CydoniaIcon.png"
		buildmessage "Copying Icon %{file.abspath}"

		buildcommands {
			"{COPY} %{file.abspath} %{cfg.targetdir}"
		}

		buildoutputs { "%{cfg.targetdir}/%{file.basename}.png" }

project "OceanDemo"
	location "Build/OceanDemo"
	language "C++"
	cppdialect "C++20"
	kind "ConsoleApp"
	architecture "x86_64"

	includedirs { "OceanDemo", "Engine", "Emporium", "include" }
	links { "Engine" }

	files { "OceanDemo/**.h",
			"OceanDemo/**.cpp",
			"CydoniaIcon.png" }

	filter {}

	filter { 'system:windows' }
		files { 'OceanDemo/OceanDemo.rc', '**.ico' }

	filter {}

	filter "files:CydoniaIcon.png"
		buildmessage "Copying Icon %{file.abspath}"

		buildcommands {
			"{COPY} %{file.abspath} %{cfg.targetdir}"
		}

		buildoutputs { "%{cfg.targetdir}/%{file.basename}.png" }

project "PBRDemo"
	location "Build/PBRDemo"
	language "C++"
	cppdialect "C++20"
	kind "ConsoleApp"
	architecture "x86_64"

	includedirs { "PBRDemo", "Engine", "Emporium", "include" }
	links { "Engine" }

	files { "PBRDemo/**.h",
			"PBRDemo/**.cpp",
			"CydoniaIcon.png" }

	filter {}

	filter { 'system:windows' }
		files { 'PBRDemo/PBRDemo.rc', '**.ico' }

	filter {}

	filter "files:CydoniaIcon.png"
		buildmessage "Copying Icon %{file.abspath}"

		buildcommands {
			"{COPY} %{file.abspath} %{cfg.targetdir}"
		}

		buildoutputs { "%{cfg.targetdir}/%{file.basename}.png" }

-- ===============================================================================================
-- Shaders
workspace "CydoniaShaders"
	location "build"
	configurations { "Release" }
	startproject "Shaders"

	objdir "intermediate"
	targetdir "bin/"

	project "Shaders"
	location "Build/Shaders"
	language "C++"
	cppdialect "C++20"
	kind "Utility"
	architecture "x86_64"

	files { "Engine/Data/Shaders/GLSL/**.comp",
			"Engine/Data/Shaders/GLSL/**.vert",
			"Engine/Data/Shaders/GLSL/**.frag",
			"Engine/Data/Shaders/GLSL/**.tesc",
			"Engine/Data/Shaders/GLSL/**.tese",
			"Engine/Data/Shaders/GLSL/**.h" }

	-- Adding Shaders Build Step
	filter { "files:Engine/Data/Shaders/GLSL/**.comp" }
		buildmessage "Compiling Compute Shader %{file.abspath}"

		buildcommands {
			"glslc -Os %{file.abspath} -o %{cfg.targetdir}/Shaders/%{file.basename}_COMP.spv"
		}

		buildoutputs { "%{cfg.targetdir}/Shaders/%{file.basename}_COMP.spv" }

	filter "files:Engine/Data/Shaders/GLSL/**.vert"
		buildmessage "Compiling Vertex Shader %{file.abspath}"

		buildcommands {
			"glslc -Os %{file.abspath} -o %{cfg.targetdir}/Shaders/%{file.basename}_VERT.spv"
		}

		buildoutputs { "%{cfg.targetdir}/Shaders/%{file.basename}_VERT.spv" }

	filter "files:Engine/Data/Shaders/GLSL/**.frag"
		buildmessage "Compiling Fragment/Pixel Shader %{file.abspath}"

		buildcommands {
			"glslc -Os %{file.abspath} -o %{cfg.targetdir}/Shaders/%{file.basename}_FRAG.spv"
		}

		buildoutputs { "%{cfg.targetdir}/Shaders/%{file.basename}_FRAG.spv" }

	filter "files:Engine/Data/Shaders/GLSL/**.tesc"
		buildmessage "Compiling Tesselation Control Shader %{file.abspath}"

		buildcommands {
			"glslc -Os %{file.abspath} -o %{cfg.targetdir}/Shaders/%{file.basename}_TESC.spv"
		}

		buildoutputs { "%{cfg.targetdir}/Shaders/%{file.basename}_TESC.spv" }

	filter "files:Engine/Data/Shaders/GLSL/**.tese"
		buildmessage "Compiling Tesselation Evaluation Shader %{file.abspath}"

		buildcommands {
			"glslc -Os %{file.abspath} -o %{cfg.targetdir}/Shaders/%{file.basename}_TESE.spv"
		}

		buildoutputs { "%{cfg.targetdir}/Shaders/%{file.basename}_TESE.spv" }