newoption {
    trigger     = "workspace",
    value       = "NAME",
    description = "Workspace/Project name (passed from Win-GenProjects.bat)"
}

local function getProjectRootName()
    local scriptDir   = path.getdirectory(_SCRIPT)
    local projectRoot = path.getabsolute(path.join(scriptDir, ".."))
    return path.getname(projectRoot) 
end

local wsName = _OPTIONS["workspace"] or getProjectRootName()

local scriptDir     = path.getdirectory(_SCRIPT)
local scriptCoreDll = path.join(scriptDir, "References", "HRealEngine-ScriptCore.dll")
local scriptCoreDir = path.getdirectory(scriptCoreDll)

workspace (wsName)
    architecture "x86_64"
    configurations { "Debug", "Release", "Dist" }
    startproject (wsName)

project (wsName)
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"

    targetdir ("Binaries")
    objdir ("Intermediates")

    files {
        "Source/**.cs",
        "Properties/**.cs"
    }

    libdirs { scriptCoreDir }
    links  { "HRealEngine-ScriptCore" }

    filter "configurations:Debug"
        optimize "Off"
        symbols "Default"

    filter "configurations:Release"
        optimize "On"
        symbols "Default"

    filter "configurations:Dist"
        optimize "Full"
        symbols "Off"
