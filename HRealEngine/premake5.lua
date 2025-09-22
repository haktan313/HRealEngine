

-- HRealEngine premake5.lua
project "HRealEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    
    pchheader "HRpch.h"
    pchsource "src/HRpch.cpp"
    
    files{
        "src/**.h",
        "src/**.cpp",
        
        "vendor/stb_image/**.h",
        "vendor/stb_image/**.cpp",
        
        "%{IncludeDir.ImGui}/backends/imgui_impl_glfw.cpp",
        "%{IncludeDir.ImGui}/backends/imgui_impl_opengl3.cpp",
        
        "vendor/glm/glm/**.hpp",
        "vendor/glm/glm/**.inl",
        
        "vendor/ImGuizmo/ImGuizmo.h",
        "vendor/ImGuizmo/ImGuizmo.cpp"
    }

    defines { "YAML_CPP_STATIC_DEFINE", "GLFW_INCLUDE_NONE", "_CRT_SECURE_NO_WARNINGS" }
    
    includedirs
    {
        "src",
        "vendor/spdlog/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGui}/backends",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.entt}",
        "%{IncludeDir['yaml-cpp']}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.Box2D}"
    }

    links
    {
        "GLFW",
        "opengl32.lib",
        "Glad",
        "ImGui",
        "yaml-cpp",
        "Box2D"
    }

    buildoptions { "/utf-8" }

    filter "system:windows"
        systemversion "latest"
        defines { "HREALENGINE_PLATFORM_WINDOWS" }
    
    filter "configurations:Debug"
        defines { "HREALENGINE_DEBUG" }
        runtime "Debug"
        symbols "on"
        staticruntime "off"
    		
    filter "configurations:Release"
        defines { "HREALENGINE_RELEASE" }
        runtime "Release"
        optimize "on"
        staticruntime "off"
    
    filter "configurations:Release"
  	    defines { "HREALENGINE_DIST" }
  	    runtime "Release"
  	    optimize "on"
  	    staticruntime "off"
  	    
  	    