-- premake5.lua
workspace "The Forge"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "App"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "The Forge"
	include "The Forge/Engine/Build-Engine.lua"
	include "The Forge/Netcode/Build-Netcode.lua"
group ""

include "App/Build-App.lua"