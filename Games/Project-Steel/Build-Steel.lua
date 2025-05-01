local scriptDir = path.getdirectory(_SCRIPT)

project "Project-Steel"
   kind "WindowedApp"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   debugdir ""
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }

   local modules = {
       "Engine",
       "Netcode"
   }

   local engine_vendor_path = "../../TheForge/Engine/Vendors/"

   includedirs
   {
      "Source",

      -- Include Engine Files
      "../../TheForge/Engine/Source",
      "../../TheForge/Netcode/Source",
      engine_vendor_path .. "glm",
      engine_vendor_path .. "imgui-1.90.5-docking",
      engine_vendor_path .. "SDL2-2.30.2/include",
      engine_vendor_path .. "glad",
      engine_vendor_path .. "stb",
      engine_vendor_path .. "nlohmann",
      engine_vendor_path .. "freetype/include"
   }
 
   prebuildcommands 
   { 
       "{DELETE} Assets/Engine Assets",
       "{COPY} \"../../TheForge/Engine/Assets\" \"Assets/Engine Assets\""
   }
   
   postbuildcommands
   {
       "{COPY} Assets/steam_appid.txt %{cfg.targetdir}",
       "{COPY} Assets %{cfg.targetdir}/Assets",
       "{COPY} Config %{cfg.targetdir}/Config",
       "{COPY} imgui.ini %{cfg.targetdir}"
   }

   -- Generate the postbuild command to copy DLLs from each module
   for _, module in ipairs(modules) do

       local copyCommand = "{COPY} "
       copyCommand = copyCommand .. "%{cfg.targetdir}/../" .. module .. "/*.dll"
       copyCommand = copyCommand .. " %{cfg.targetdir}"

       postbuildcommands { copyCommand }

       local assetCopy = "{COPY} "
       assetCopy = assetCopy .. "%{cfg.targetdir}/../" .. module .. "/Assets"
       assetCopy = assetCopy .. "%{cfg.targetdir}/Assets"

       postbuildcommands { assetCopy }

       local deleteCommand = "{DELETE} "
       deleteCommand = deleteCommand .. "%{cfg.targetdir}/../" .. module .. "/*.dll"

       postbuildcommands { deleteCommand }
       
   end

   targetdir ("../../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }
       links { modules }
       entrypoint "mainCRTStartup"

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