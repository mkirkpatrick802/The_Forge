-- (Removed a recursive "del /S /Q *.vcxproj" that ran at include time against the
--  current working directory. See the note at the top of Build.lua.)

project "Netcode"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }

   includedirs
   {
      "Source",
      "Vendors/SteamSDK/include",
      "../Engine/Source",
      "../Engine/Vendors/nlohmann",
      "../Engine/Vendors/glm"
   }

   targetdir ("../../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }
       libdirs { "Vendors/SteamSDK/bin", "Vendors/SteamSDK/bin/win64"}

       -- ws2_32 is Winsock, used by the UDP transport under Source/Transport.
       links   { "steam_api",  "steam_api64", "ws2_32"}

       postbuildcommands 
       {
            "{COPY} Vendors/SteamSDK/bin/win64/*.dll %{cfg.targetdir}",
            "{COPY} Vendors/SteamSDK/bin/*.dll %{cfg.targetdir}",
            "{COPY} Assets/steam_appid.txt %{cfg.targetdir}"
       }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"