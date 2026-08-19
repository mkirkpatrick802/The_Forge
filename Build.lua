-- premake5.lua
--
-- This script used to begin by deleting Binaries/ and recursively deleting every
-- *.sln from the working directory. Both were removed 2026-08-17:
--   * the deletes were unanchored -- they acted on whatever the current working
--     directory happened to be, not on this file's location;
--   * "/S" made them recursive, so they could reach vendor or unrelated projects;
--   * wiping Binaries forced a full rebuild after every generation, and silently
--     destroyed the outputs of projects that were not about to be rebuilt.
-- Premake overwrites the files it generates, so none of it was necessary.
-- Use Scripts/Clean.bat when a genuinely clean slate is wanted.

workspace "The Forge"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   -- The Launcher is still in development, so F5 should run the game.
   startproject "Project-Steel"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "The Forge Engine"
	include "TheForge/Engine/Build-Engine.lua"
	include "TheForge/Netcode/Build-Netcode.lua"
group ""

group "The Forge Launcher"
	include "Launcher/Build-Launcher.lua"
group ""

include "Games/Build-Games.lua"
