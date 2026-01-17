project "BehaviorTreeLibrary"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src/**.h",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src/**.hpp",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src/**.cpp",

        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/libs/imgui-node-editor/imgui_canvas.cpp",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/libs/imgui-node-editor/imgui_node_editor.cpp",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/libs/imgui-node-editor/imgui_node_editor_api.cpp",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/libs/imgui-node-editor/crude_json.cpp",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/libs/imgui-node-editor/**.h"
    }

    includedirs
    {
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src/BehaviorTreeThings",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src/BehaviorTreeThings/Core",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src/BehaviorTreeThings/CustomThings",
        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/src/BehaviorTreeThings/Editor",

        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGui}/backends",
        "%{IncludeDir.glm}",
        "%{IncludeDir['yaml-cpp']}",
        "%{wks.location}/HRealEngine/vendor/spdlog/include",

        "%{wks.location}/HRealEngine/vendor/BehaviorTreeLibrary/libs/imgui-node-editor",

        "%{wks.location}/HRealEngine/src",
        "%{wks.location}/HRealEngine/vendor"
    }

    defines
    {
        "YAML_CPP_STATIC_DEFINE"
    }

    links
    {
        "yaml-cpp",
        "ImGui",
        "Glad",
        "GLFW",
        "opengl32.lib"
    }

    

    filter "system:windows"
        systemversion "latest"
        defines { "HREALENGINE_PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        runtime "Release"
        optimize "on"
