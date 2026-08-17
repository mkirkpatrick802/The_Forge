project "Launcher"
   kind "WindowedApp"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }

   -- Engine now references NetCode (byte streams, LinkingContext, NetworkManager),
   -- so the Launcher has to link it too even though it has no netcode of its own.
   local modules = {
       "Engine",
       "Netcode"
   }

   local engine_vendor_path = "../TheForge/Engine/Vendors/"

   includedirs
   {
      "Source",

      -- Include Engine Files
      "../TheForge/Engine/Source",
      engine_vendor_path .. "glm",
      engine_vendor_path .. "imgui-1.90.5-docking",
      engine_vendor_path .. "SDL2-2.30.2/include",
      engine_vendor_path .. "glad",
      engine_vendor_path .. "stb",
   }

   -- The engine loads its own assets from "Assets/Engine Assets/..." (e.g. the
   -- ScreenQuad shader), so they must land in that subfolder rather than flat in
   -- Assets/. Deleting only that subfolder also leaves the Launcher's own
   -- committed assets alone.
   prebuildcommands
   {
       "{DELETE} Assets/Engine Assets",
       "{COPY} \"../TheForge/Engine/Assets\" \"Assets/Engine Assets\""
   }

   postbuildcommands
   {
       "{COPY} Assets %{cfg.targetdir}/Assets"
   }

   -- Generate the postbuild command to copy DLLs from each module
   for _, module in ipairs(modules) do

       local copyCommand = "{COPY} "
       copyCommand = copyCommand .. "%{cfg.targetdir}/../" .. module .. "/*.dll"
       copyCommand = copyCommand .. " %{cfg.targetdir}"

       postbuildcommands { copyCommand }

       local assetCopy = "{COPY} "
       assetCopy = assetCopy .. "%{cfg.targetdir}/../" .. module .. "/Assets"
       assetCopy = assetCopy .. " %{cfg.targetdir}/Assets"

       postbuildcommands { assetCopy }

       -- See Build-Steel.lua: the module's DLLs are deliberately not deleted after
       -- copying, since both this and the game copy from the same module dir.
   end

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

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