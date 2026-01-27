-- premake5.lua (Scripts/premake5.lua)
-- Generates a C# class library that outputs Sandbox.dll into Scripts/Binaries

newoption {
    trigger     = "workspace",
    value       = "NAME",
    description = "Workspace name"
}

newoption {
    trigger     = "scriptcore",
    value       = "PATH",
    description = "Path to HRealEngine-ScriptCore.dll (optional override)"
}

local wsName = _OPTIONS["workspace"] or "SandboxProject"

-- Default: Scripts/References/HRealEngine-ScriptCore.dll (relative to this premake file)
local scriptDir = path.getdirectory(_SCRIPT)
local defaultScriptCore = path.join(scriptDir, "References", "HRealEngine-ScriptCore.dll")
local scriptCoreDll = _OPTIONS["scriptcore"] or defaultScriptCore
local scriptCoreDir = path.getdirectory(scriptCoreDll)

workspace (wsName)
    architecture "x86_64"
    startproject (wsName)

    configurations { "Debug", "Release", "Dist" }
    flags { "MultiProcessorCompile" }

project (wsName)
    kind "SharedLib"-- C# class library (.dll)
    language "C#"
    dotnetframework "4.7.2"

    targetdir ("Binaries")
    objdir ("Intermediates")

    files {
        "Source/**.cs",
        "Properties/**.cs"
    }

    -- Reference ScriptCore DLL (no engine source dependency)
    libdirs { scriptCoreDir }
    links { "HRealEngine-ScriptCore" }

    filter "configurations:Debug"
        optimize "Off"
        symbols "Default"

    filter "configurations:Release"
        optimize "On"
        symbols "Default"

    filter "configurations:Dist"
        optimize "Full"
        symbols "Off"
