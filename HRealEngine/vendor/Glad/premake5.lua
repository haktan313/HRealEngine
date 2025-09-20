

project "Glad"
	kind "StaticLib"
	language "C"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/glad/glad.h",
		"include/KHR/khrplatform.h",
		"src/glad.c"
	}

	includedirs
	{
		"include"
	}

	filter "system:windows"
		systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        --buildoptions "/MDd"
		--buildoptions "/MTd"
        symbols "on"
    
    filter "configurations:Release"
        runtime "Release"
        --buildoptions "/MD"
		--buildoptions "/MT"
        optimize "on"


