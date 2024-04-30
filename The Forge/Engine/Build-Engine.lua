project "Engine"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }
   files { "Vendors/glad/**.h", "Vendors/glad/**.c"}
   files { "Vendors/imgui-1.90.5-docking/**.h", "Vendors/imgui-1.90.5-docking/**.cpp"}

   includedirs
   {
      "Source",

      "Vendors/SDL2-2.30.2/include",
      "Vendors/imgui-1.90.5-docking",
      "vendors/glad",
      "Vendors/glm"
   }

   targetdir ("../../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }
       libdirs { "Vendors/SDL2-2.30.2/lib/x64" }
       links   { "SDL2", "SDL2main" }

       
       postbuildcommands 
       {
            "{COPY} Vendors/SDL2-2.30.2/lib/x64/*.dll %{cfg.targetdir}"
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