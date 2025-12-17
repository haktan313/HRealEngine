
--HRealEngine Editor premake5.lua
project "HRealEngine Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    
    files{"src/**.h", "src/**.cpp"}
    includedirs
    {
        "%{wks.location}/HRealEngine/vendor/spdlog/include",
        "%{wks.location}/HRealEngine/src",
        "%{wks.location}/HRealEngine/vendor",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.mono}",
        "%{IncludeDir.filewatch}",
        "%{IncludeDir.JoltPhysics}"
    }

    buildoptions { "/utf-8" }
    
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
    		
    		
    		