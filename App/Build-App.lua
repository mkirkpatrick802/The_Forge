project "App"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }

   includedirs
   {
      "Source",

      -- Include Engine Files
      "../The Forge/Engine/Source",
      "../The Forge/Netcode/Source"
   }

   local modules = {
       "Engine",
       "Netcode"
   }

   -- Generate the postbuild command to copy DLLs from each module
   for _, module in ipairs(modules) do
       local copyCommand = "{COPY} "
       copyCommand = copyCommand .. "%{cfg.targetdir}/../" .. module .. "/*.dll"
       copyCommand = copyCommand .. " %{cfg.targetdir}"

       postbuildcommands { copyCommand }
   end

   -- Generate the command to delete DLLs from the original module paths
   for _, module in ipairs(modules) do
       local deleteCommand = "{DELETE} "
       deleteCommand = deleteCommand .. "%{cfg.targetdir}/../" .. module .. "/*.dll"

       postbuildcommands { deleteCommand }
   end

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }
       links { modules }

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