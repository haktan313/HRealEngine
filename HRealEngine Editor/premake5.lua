
--HRealEngine Editor premake5.lua
project "HRealEngine Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    
    files{"src/**.h", "src/**.cpp"}
    includedirs{
        "%{wks.location}/HRealEngine/vendor/spdlog/include",
        "%{wks.location}/HRealEngine/src",
        "%{wks.location}/HRealEngine/vendor",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.mono}",
        "%{IncludeDir.filewatch}",
        "%{IncludeDir.JoltPhysics}",

        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src/BehaviorTreeThings",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src/BehaviorTreeThings/Core",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src/BehaviorTreeThings/CustomThings",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src/BehaviorTreeThings/Editor",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/libs/imgui-node-editor",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir['yaml-cpp']}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.GLFW}"
    }

    buildoptions { "/utf-8" }

    defines{
        "YAML_CPP_STATIC_DEFINE"
    }


    postbuildcommands{
        '{COPYFILE} "%{wks.location}/HRealEngine Editor/imgui.ini" "%{cfg.targetdir}"',
        '{COPYDIR} "%{wks.location}/HRealEngine Editor/mono" "%{cfg.targetdir}/mono"',
        '{COPYDIR} "%{wks.location}/HRealEngine Editor/assets" "%{cfg.targetdir}/assets"',
        '{COPYDIR} "%{wks.location}/HRealEngine Editor/Resources" "%{cfg.targetdir}/Resources"',
        '{COPYDIR} "%{wks.location}/HRealEngine Editor/SandboxProject/Assets/Scripts/Binaries" "%{cfg.targetdir}/SandboxProject/Assets/Scripts/Binaries"'
    }

    
    links{"HRealEngine"}
    
    	filter "system:windows"
    		systemversion "latest"
    		defines { "HREALENGINE_PLATFORM_WINDOWS" }
    
    	filter "configurations:Debug"
    		defines { "HREALENGINE_DEBUG", "JPH_DEBUG", "JPH_ENABLE_ASSERTS" }
    		runtime "Debug"
    		symbols "on"
    
    	filter "configurations:Release"
    		defines { "HREALENGINE_RELEASE", "JPH_RELEASE" }
    		runtime "Release"
    		optimize "on"
    
    	filter "configurations:Dist"
    		defines { "HREALENGINE_DIST", "JPH_DIST" }
    		runtime "Release"
    		optimize "on"
    		
    		
    		