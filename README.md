# HRealEngine

**HRealEngine** is a custom **2D Game Engine** built with **C++** and **OpenGL**.

- Start with cloning the repo with `git clone --recursive https://github.com/haktan313/HRealEngine`

- If you cloned it without the `--recursive` flag, initialize submodules manually `git submodule update --init`

- If you make changes to the build files or need to regenerate Visual Studio project files, run `scripts/Win-GenProjects.bat` this will call **premake5** and create the required `.sln` files.

## 📸 Screenshots
| Editor | Sample Scene |
|--------|-------------|
| ![Editor Screenshot](https://github.com/user-attachments/assets/db5d561c-ae5e-4dee-8438-a36c4970f981) | ![Scene Screenshot](https://github.com/user-attachments/assets/11c43985-ec0e-4c48-8372-daf0f393670a) |

I used a few submodules there is:

## 📦 Third-Party Submodules

HRealEngine uses the following libraries (all included as Git submodules):

- **[Box2D](https://github.com/erincatto/box2d)** – 2D physics engine
- **[entt](https://github.com/skypjack/entt)** – Entity Component System
- **[GLFW](https://github.com/glfw/glfw)** – Window and input handling
- **[Glad](https://github.com/Dav1dde/glad)** – OpenGL function loader
- **[ImGui](https://github.com/ocornut/imgui)** – Immediate mode GUI
- **[ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)** – Gizmo manipulation for ImGui
- **[spdlog](https://github.com/gabime/spdlog)** – Fast logging library
- **[stb_image](https://github.com/nothings/stb)** – Image loading
- **[yaml-cpp](https://github.com/jbeder/yaml-cpp)** – Serialization
- **[glm](https://github.com/g-truc/glm)** – Math library for graphics
