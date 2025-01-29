-- premake5.lua
os.execute('rmdir /S /Q Binaries 2>nul')

workspace "The Forge"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Launcher"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "The Forge Engine"
	include "The Forge/Engine/Build-Engine.lua"
	include "The Forge/Netcode/Build-Netcode.lua"
group ""

group "The Forge Launcher"
	include "Launcher/Build-Launcher.lua"
group ""

include "Games/Build-Games.lua"
