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
      "../Engine/Source"
   }

   targetdir ("../../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }
       libdirs { "Vendors/SteamSDK/bin", "Vendors/SteamSDK/bin/win64"}
       links   { "steam_api",  "steam_api64"}

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