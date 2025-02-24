os.execute('del /S /Q "*.vcxproj" 2>nul')

project "Engine"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp", "Source/**.inl" }
   files { "Vendors/glad/**.h", "Vendors/glad/**.c"}
   files { "Vendors/imgui-1.90.5-docking/**.h", "Vendors/imgui-1.90.5-docking/**.cpp"}
   files { "Vendors/stb/**.h"}
   files { "Vendors/nlohmann/**.hpp"}
   files { "Vendors/freetype/include/**.h"}

   includedirs
   {
      "Source",
      "../Netcode/Source",

      "Vendors/SDL2-2.30.2/include",
      "Vendors/imgui-1.90.5-docking",
      "Vendors/glad",
      "Vendors/glm",
      "Vendors/stb",
      "Vendors/nlohmann",
      "Vendors/freetype/include"
   }

   targetdir ("../../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }
       libdirs { "Vendors/SDL2-2.30.2/lib/x64", "Vendors/freetype/release dll/win64" }
       links   { "SDL2", "SDL2main", "SDL2_image", "freetype" }

       postbuildcommands
       {
            "{COPY} Vendors/SDL2-2.30.2/lib/x64/*.dll %{cfg.targetdir}",
            "{COPY} \"Vendors/freetype/release dll/win64/*.dll\" %{cfg.targetdir}",
            "{COPY} Assets %{cfg.targetdir}/Assets"   
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