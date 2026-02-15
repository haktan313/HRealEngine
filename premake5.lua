include "Dependencies.lua"

workspace "HRealEngine"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }
    startproject "HRealEngine Editor"
    
    	--solution_items{".editorconfig"}
    	flags{"MultiProcessorCompile"}
    	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    	
    	filter "files:HRealEngine/vendor/**.c"
            flags { "NoPCH" }
        filter "files:HRealEngine/vendor/**.cpp"
            flags { "NoPCH" }

    group "Dependencies"
    	include "HRealEngine/vendor/GLFW"
    	include "HRealEngine/vendor/Glad"
        include "HRealEngine/vendor/msdf-atlas-gen"
    	include "HRealEngine/vendor/imgui"
    	include "HRealEngine/vendor/yaml-cpp"
    	include "HRealEngine/vendor/Box2D"
        include "HRealEngine/vendor/JoltPhysics"
        include "HRealEngine/vendor/assimp"
        include "HRealEngine/BehaviorTreeLibrary_premake5.lua"
    group ""
    include "HRealEngine"
    include "HRealEngine Editor"
    include "HRealEngine Runtime"
    include "HRealEngine-ScriptCore"
    
    
    