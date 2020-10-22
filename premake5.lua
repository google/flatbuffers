-- flatbuffers/premake5.lua

	-------------------------------
	-- [ PROJECT CONFIGURATION ] --
	-------------------------------
	project "Flatbuffers"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"
		targetdir ("%{prj.location}/bin/%{cfg.platform}/%{cfg.buildcfg}")
		objdir "%{prj.location}/obj/%{prj.name}/%{cfg.platform}/%{cfg.buildcfg}"
		defines { 'PATH_REFLECTION_FBS="' .. path.getabsolute("reflection/reflection.fbs") .. '"' }

		local incDir = "include/"

		files
		{
			incDir .. "**.h",
			"src/idl_parser.cpp",
			"src/idl_gen_text.cpp",
			"src/util.cpp",
			"src/reflection.cpp",
			"reflection/reflection.fbs"
		}

		includedirs { incDir }