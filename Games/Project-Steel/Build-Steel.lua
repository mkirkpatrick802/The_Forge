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
   
   -- Assets and Config are required to run, so they stay as {COPY}: if either is
   -- missing the build should fail loudly. steam_appid.txt and imgui.ini are
   -- optional -- the first only matters in Steam mode, and ImGui recreates the
   -- second -- so a missing one must not fail the build.
   postbuildcommands
   {
       "{COPY} Assets %{cfg.targetdir}/Assets",
       "{COPY} Config %{cfg.targetdir}/Config",
       'if exist "Assets\\steam_appid.txt" xcopy /Q /Y /I "Assets\\steam_appid.txt" "..\\..\\Binaries\\%{cfg.system}-%{cfg.architecture}\\%{cfg.buildcfg}\\%{prj.name}" > nul',
       'if exist "imgui.ini" xcopy /Q /Y /I "imgui.ini" "..\\..\\Binaries\\%{cfg.system}-%{cfg.architecture}\\%{cfg.buildcfg}\\%{prj.name}" > nul'
   }

   -- Copy each module's DLLs, and any assets it ships, out of that module's output
   -- directory.
   --
   -- Both are guarded with "if exist" rather than premake's {COPY}: {COPY} always
   -- invokes xcopy, which fails the whole build (MSB3073, xcopy exit code 4) when
   -- the source is missing. That is a normal state here -- Netcode ships no Assets
   -- folder, and a module may simply not have been built yet.
   --
   -- The module's DLLs are also deliberately NOT deleted after copying: every
   -- consumer (this game, the Launcher) copies from the same module output dir, so
   -- deleting them starves whichever project builds second.
   -- Paths are spelled with backslashes rather than reusing %{cfg.targetdir}, which
   -- expands with forward slashes. cmd's "if exist" silently evaluates false against
   -- a forward-slash wildcard, which would turn every copy below into a no-op and
   -- produce a clean build with no DLLs beside the exe.
   local binRoot = "..\\..\\Binaries\\%{cfg.system}-%{cfg.architecture}\\%{cfg.buildcfg}"

   for _, module in ipairs(modules) do

       local moduleDir = binRoot .. "\\" .. module
       local targetDir = binRoot .. "\\%{prj.name}"

       postbuildcommands
       {
           'if exist "' .. moduleDir .. '\\*.dll" xcopy /Q /Y /I "' .. moduleDir .. '\\*.dll" "' .. targetDir .. '" > nul',
           'if exist "' .. moduleDir .. '\\Assets" xcopy /Q /E /Y /I "' .. moduleDir .. '\\Assets" "' .. targetDir .. '\\Assets" > nul'
       }
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