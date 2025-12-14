

--Dependency Include Directories
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/HRealEngine/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/HRealEngine/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/HRealEngine/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/HRealEngine/vendor/glm"
IncludeDir["stb_image"] = "%{wks.location}/HRealEngine/vendor/stb_image"
IncludeDir["entt"] = "%{wks.location}/HRealEngine/vendor/entt/include"
IncludeDir["yaml-cpp"] = "%{wks.location}/HRealEngine/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/HRealEngine/vendor/ImGuizmo"
IncludeDir["Box2D"] = "%{wks.location}/HRealEngine/vendor/Box2D/include"
IncludeDir["mono"] = "%{wks.location}/HRealEngine/vendor/mono/include"
IncludeDir["filewatch"] = "%{wks.location}/HRealEngine/vendor/filewatch"
IncludeDir["JoltPhysics"] = "%{wks.location}/HRealEngine/vendor/JoltPhysics"
IncludeDir["Jolt"] = "%{wks.location}/HRealEngine/vendor/JoltPhysics/Jolt"


LibraryDir = {}
LibraryDir["mono"] = "%{wks.location}/HRealEngine/vendor/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"
 
--Windows DLLs
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["Bcrypt"] = "Bcrypt.lib"


