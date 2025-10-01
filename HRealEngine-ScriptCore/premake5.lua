

-- HRealEngine premake5.lua
project "HRealEngine-ScriptCore"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"
    
    targetdir ("../HRealEngine Editor/Resources/Scripts")
    objdir ("../HRealEngine Editor/Resources/Scripts/Intermediates")
    
    files
    {
        "Source/**.cs",   
        "Properties/**.cs"
    }

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"
    
    filter "configurations:Release"
        optimize "On"
        symbols "Default"
        
    filter "system:Dist"
        optimize "Full"
        symbols "Off"