
//EditorLayer.cpp
#include "EditorLayer.h"
#include "glm/gtc/type_ptr.hpp"
#include <chrono>
#include "HRpch.h"
#include "HRealEngine/Core/Application.h"
#include "HRealEngine/Core/Input.h"
#include "HRealEngine/Core/MouseButtonCodes.h"
#include "HRealEngine/Renderer/RenderCommand.h"
#include "HRealEngine/Renderer/Renderer2D.h"
#include "HRealEngine/Scene/SceneSerializer.h"
#include "HRealEngine/Utils/PlatformUtils.h"
#include "imgui/imgui.h"
#include "ImGuizmo/ImGuizmo.h"


namespace HRealEngine
{
    extern const std::filesystem::path g_AssetsDirectory;
    
    EditorLayer::EditorLayer() : Layer("EditorLayer"), orthCameraControllerRef(1280.0f / 720.0f, true)
    {
        
    }

    void EditorLayer::OnAttach()
    {
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        framebufferRef = Framebuffer::Create(fbSpec);
        
        /*joseMourinhoTextureRef = Texture2D::Create("assets/textures/joseMourinho.png");
        checkBoardTextureRef = Texture2D::Create("assets/textures/Checkerboard.png");
        spriteSheetRef = Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

        textureStairs = SubTexture2D::CreateFromCoords(spriteSheetRef, {7, 6}, {128, 128});
        textureTree = SubTexture2D::CreateFromCoords(spriteSheetRef, {2, 1}, {128, 128}, {1,2});
        textureBarrel = SubTexture2D::CreateFromCoords(spriteSheetRef, {8, 2}, {128, 128});

        m_Particle.ColorBegin = { 254 / 255.f, 212 / 255.f, 123 / 255.f, 1.0f };
        m_Particle.ColorEnd = { 254 / 255.f, 109 / 255.f, 41 / 255.f, 1.0f };
        m_Particle.SizeBegin = 0.5f;
        m_Particle.SizeVariation = 0.3f;
        m_Particle.SizeEnd = 0.0f;
        m_Particle.LifeTime = 1.0f;
        m_Particle.Velocity = { 0.0f, 0.0f };
        m_Particle.VelocityVariation = { 3.0f, 1.0f };
        m_Particle.Position = { 0.0f, 0.0f };*/

        activeSceneRef = CreateRef<Scene>();
        m_EditorCamera = EditorCamera(30.f, 1.778f/*1920/1080*/, 0.1f, 1000.f);
        
        /*
        cameraEntity = activeSceneRef->CreateEntity("Camera Entity");
        cameraEntity.AddComponent<CameraComponent>();
        class CameraController : public ScriptableEntity
        {
        public:
            void OnCreate()
            {
                std::cout << "CameraController::OnCreate" << std::endl;
            }
            void OnDestroy()
            {
                
            }
            void OnUpdate(Timestep ts)
            {
                auto& transformComponent = GetComponent<TransformComponent>();
                float speed = 5.f;

                if (Input::IsKeyPressed(HR_KEY_A))
                    transformComponent.Position.x -= speed * ts.GetSeconds();
                if (Input::IsKeyPressed(HR_KEY_D))
                    transformComponent.Position.x += speed * ts.GetSeconds();
                if (Input::IsKeyPressed(HR_KEY_W))
                    transformComponent.Position.y += speed * ts.GetSeconds();
                if (Input::IsKeyPressed(HR_KEY_S))
                    transformComponent.Position.y -= speed * ts.GetSeconds();
            }
        };
        cameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
        */
        iconPlayRef = Texture2D::Create("assets/textures/StartButton.png");
        iconStopRef = Texture2D::Create("assets/textures/stopButton.png");
        sceneHierarchyPanelRef.SetContext(activeSceneRef);
    }

    void EditorLayer::OnDetach()
    {

    }

    void EditorLayer::OnUpdate(Timestep timestep)
    {
        HR_PROFILE_FUNC();

        if (FramebufferSpecification spec = framebufferRef->GetSpecification();
            m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            framebufferRef->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            orthCameraControllerRef.OnResize(m_ViewportSize.x, m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
            activeSceneRef->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }
        
        {
            if (m_ViewportFocused)
                orthCameraControllerRef.OnUpdate(timestep);
            m_EditorCamera.OnUpdate(timestep);
        }
        
        Renderer2D::ResetStats();
        {
            HR_PROFILE_SCOPE("Renderer Prep");
            framebufferRef->Bind();
            RenderCommand::Clear();
            RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        }
        framebufferRef->ClearAttachment(1, -1);
        /*{
            HR_PROFILE_SCOPE("Renderer Draw");
            Renderer2D::BeginScene(orthCameraControllerRef.GetCamera());

            Renderer2D::DrawRotatedQuad({-0.2f, 0.5f}, {0.8f, 0.8f}, glm::radians(-75.f), {0.8f, 0.2f, 0.3f, 1.0f});
            Renderer2D::DrawQuad({-0.7f, 0.0f}, {0.8f, 0.4f}, {0.2f, 0.8f, 0.3f, 1.0f});
            Renderer2D::DrawQuad({0.5f, -0.5f}, {1.f, 1.0f}, joseMourinhoTextureRef);
            Renderer2D::DrawRotatedQuad({0.f, 0.f}, {1.f, 1.f}, glm::radians(45.f), checkBoardTextureRef, 30.f, glm::vec4(.5f, 0.3f, 0.9f, 1.0f));
            Renderer2D::DrawQuad({0.0f, 0.0f}, {10.f, 10.f}, checkBoardTextureRef, 10.f);
        
            Renderer2D::EndScene();
        }*/

        /*if (Input::IsMouseButtonPressed(HR_MOUSE_BUTTON_LEFT))
        {
            auto [x, y] = Input::GetMousePosition();
            auto width = Application::Get().GetWindow().GetWidth();
            auto height = Application::Get().GetWindow().GetHeight();

            auto bounds = orthCameraControllerRef.GetBounds();
            auto pos = orthCameraControllerRef.GetCamera().GetPosition();
            x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
            y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
            m_Particle.Position = { x + pos.x, y + pos.y };
            for (int i = 0; i < 5; i++)
                m_ParticleSystem.Emit(m_Particle);
        }
    
        m_ParticleSystem.OnUpdate(timestep);
        m_ParticleSystem.OnRender(orthCameraControllerRef.GetCamera());

        Renderer2D::BeginScene(orthCameraControllerRef.GetCamera());
        Renderer2D::DrawQuad({0.f,2.f, 0.5f}, {1.f,1.f}, textureStairs);
        Renderer2D::DrawQuad({-1.f,0.f, 0.5f}, {1.f,2.f}, textureTree);
        Renderer2D::DrawQuad({1.f,0.f, 0.5f}, {1.f,1.f}, textureBarrel);
        Renderer2D::EndScene();*/

        //Renderer2D::BeginScene(orthCameraControllerRef.GetCamera());
        //activeSceneRef->OnUpdateRuntime(timestep);
        //activeSceneRef->OnUpdateEditor(timestep, m_EditorCamera);
        if (m_SceneState == SceneState::Runtime)
            activeSceneRef->OnUpdateRuntime(timestep);
        else if (m_SceneState == SceneState::Editor)
        {
            if (m_ViewportFocused)
                orthCameraControllerRef.OnUpdate(timestep);
            m_EditorCamera.OnUpdate(timestep);
            activeSceneRef->OnUpdateEditor(timestep, m_EditorCamera);
        }
        //Renderer2D::EndScene();

        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
        my = viewportSize.y - my;

        int mouseX = (int)mx;
        int mouseY = (int)my;
        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
        {
            int pixelData = framebufferRef->ReadPixel(1, mouseX, mouseY);
            m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, activeSceneRef.get());
            LOG_CORE_INFO("Pixel data = {0}", pixelData);
        }
        
        framebufferRef->Unbind();
    }

    void EditorLayer::OnImGuiRender()
    {
                      // READ THIS !!!
        // TL;DR; this demo is more complicated than what most users you would normally use.
        // If we remove all options we are showcasing, this demo would become:
        //     void ShowExampleAppDockSpace()
        //     {
        //         ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
        //     }
        // In most cases you should be able to just call DockSpaceOverViewport() and ignore all the code below!
        // In this specific demo, we are not using DockSpaceOverViewport() because:
        // - (1) we allow the host window to be floating/moveable instead of filling the viewport (when opt_fullscreen == false)
        // - (2) we allow the host window to have padding (when opt_padding == true)
        // - (3) we expose many flags and need a way to have them visible.
        // - (4) we have a local menu bar in the host window (vs. you could use BeginMainMenuBar() + DockSpaceOverViewport()
        //      in your code, but we don't here because we allow the window to be floating)
    
        static bool p_open = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    
        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }
    
        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;
    
        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &p_open, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();
    
        if (opt_fullscreen)
            ImGui::PopStyleVar(2);
    
        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 320.0f; // Make the host window not too small
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }
        style.WindowMinSize.x = minWinSizeX;
    
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                if (ImGui::MenuItem("New", "Ctrl+N"))
                {
                   NewScene();
                }
                if (ImGui::MenuItem("Open...", "Ctrl+0"))
                {
                    OpenScene();
                }
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                {
                    SaveSceneAs();
                }
                if (ImGui::MenuItem("Exit"))
                    HRealEngine::Application::Get().Close();
                
                ImGui::EndMenu();
            }
    
            ImGui::EndMenuBar();
        }

        sceneHierarchyPanelRef.OnImGuiRender();
        contentBrowserPanelRef.OnImGuiRender();
        
        ImGui::Begin("Profile Results");

        auto name = m_HoveredEntity ? m_HoveredEntity.GetComponent<TagComponent>().Tag : "None";
        ImGui::Text("Hovered Entity: %s", name.c_str());
        
        auto stats = Renderer2D::GetStats();
        ImGui::Text("Renderer2D Stats:");
        ImGui::Text("Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Quads: %d", stats.QuadCount);
        ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
        ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        ImGui::Begin("Viewport");

        auto viewportOffset = ImGui::GetCursorPos();
        
        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered);
        
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize) && viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
        {
            //framebufferRef->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
            m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};
            //orthCameraControllerRef.OnResize(m_ViewportSize.x, m_ViewportSize.y);
        }

        uint32_t textureID = framebufferRef->GetColorAttachmentRendererID();
        ImGui::Image((void*)textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0, 1), ImVec2(1, 0));

        if (ImGui::BeginDragDropTarget())
        {
            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                OpenScene(std::filesystem::path(g_AssetsDirectory) / path);
            }
            ImGui::EndDragDropTarget();
        }

        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 minBound = ImGui::GetWindowPos();
        minBound.x += viewportOffset.x;
        minBound.y += viewportOffset.y;
        ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
        m_ViewportBounds[0] = { minBound.x, minBound.y };
        m_ViewportBounds[1] = { maxBound.x, maxBound.y };

        Entity selectedEntity = sceneHierarchyPanelRef.GetSelectedEntity();
        if (selectedEntity && m_GizmoType != -1)
        {
            ImGuizmo::SetOrthographic(true);
            ImGuizmo::SetDrawlist();
            float windowWidth = (float)ImGui::GetWindowWidth();
            float windowHeight = (float)ImGui::GetWindowHeight();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

            /*auto cameraEntity = activeSceneRef->GetPrimaryCameraEntity();
            const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
            const glm::mat4& cameraProjection = camera.GetProjectionMatrix();
            glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());*/

            const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
            glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

            auto& transformComponent = selectedEntity.GetComponent<TransformComponent>();
            glm::mat4 transform = transformComponent.GetTransform();

            bool bSnap = Input::IsKeyPressed(HR_KEY_LEFT_CONTROL) || Input::IsKeyPressed(HR_KEY_RIGHT_CONTROL);
            float snapValue = m_GizmoType == ImGuizmo::OPERATION::ROTATE ? 45.f : 0.5f;

            float snapValues[3] = { snapValue, snapValue, snapValue };

            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::MODE::LOCAL, glm::value_ptr(transform),
                nullptr, bSnap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                glm::vec3 translation, rotation, scale;
                activeSceneRef->DecomposeTransform(transform, translation, rotation, scale);
                transformComponent.Position = translation;
                glm::vec3 deltaRotation = rotation - transformComponent.Rotation;
                transformComponent.Rotation += deltaRotation;
                transformComponent.Scale = scale;
            }
        }
        
        ImGui::End();
        ImGui::PopStyleVar();

        UIToolbar();
        
        ImGui::End();
    }

    void EditorLayer::OnEvent(EventBase& eventRef)
    {
        orthCameraControllerRef.OnEvent(eventRef);
        m_EditorCamera.OnEvent(eventRef);

        EventDispatcher dispatcher(eventRef);
        dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& event)
    {
        if (event.GetRepeatCount() > 0)
            return false;
        bool bControlPressed = Input::IsKeyPressed(HR_KEY_LEFT_CONTROL) || Input::IsKeyPressed(HR_KEY_RIGHT_CONTROL);
        bool bShiftPressed = Input::IsKeyPressed(HR_KEY_LEFT_SHIFT) || Input::IsKeyPressed(HR_KEY_RIGHT_SHIFT);
        switch (event.GetKeyCode())
        {
        case HR_KEY_N:
            if (bControlPressed)
                NewScene();
            break;
        case HR_KEY_O:
            if (bControlPressed)
                OpenScene();
            break;
        case HR_KEY_S:
            if (bControlPressed)
                if (bShiftPressed)
                    SaveSceneAs();
                else
                    SaveScene();
            break;
        case HR_KEY_D:
            if (bControlPressed)
                OnDuplicateEntity();
            break;
            
        case HR_KEY_Q:
            m_GizmoType = -1;
            break;
        case HR_KEY_W:
            m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
            break;
        case HR_KEY_E:
            m_GizmoType = ImGuizmo::OPERATION::ROTATE;
            break;
        case HR_KEY_R:
            m_GizmoType = ImGuizmo::OPERATION::SCALE;
            break;

        }
        return false;
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& event)
    {
        if (event.GetMouseButton() == HR_MOUSE_BUTTON_LEFT)
            if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(HR_KEY_LEFT_ALT))
                sceneHierarchyPanelRef.SetSelectedEntity(m_HoveredEntity);
        return false;
    }

    void EditorLayer::NewScene()
    {
        activeSceneRef = CreateRef<Scene>();
        activeSceneRef->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        sceneHierarchyPanelRef.SetContext(activeSceneRef);

        editorScenePath = std::filesystem::path();
    }

    void EditorLayer::OpenScene()
    {
        std::string filePath = FileDialogs::OpenFile("HRE Scene (*.hrs)\0*.hrs\0");
        if (!filePath.empty())
            OpenScene(filePath);
    }

    void EditorLayer::OpenScene(const std::filesystem::path& path)
    {
        if (m_SceneState != SceneState::Editor)
            OnSceneStop();
        
        if (path.extension().string() != ".hrs")
        {
            LOG_CORE_ERROR("Could not load {0} - not a .hrs file", path.filename().string());
            return;
        }
        Ref<Scene> newScene = CreateRef<Scene>();
        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(path.string()))
        {
            editorSceneRef = newScene;
            editorSceneRef->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            sceneHierarchyPanelRef.SetContext(editorSceneRef);

            activeSceneRef = editorSceneRef;
            editorScenePath = path;
        }
    }

    void EditorLayer::SaveSceneAs()
    {
        std::string filePath = FileDialogs::SaveFile("HRE Scene (*.hrs)\0*.hrs\0");
        if (!filePath.empty())
        {
            SerializeScene(activeSceneRef, filePath);
            editorScenePath = filePath;
        }
    }

    void EditorLayer::SaveScene()
    {
        if (!editorScenePath.empty())
            SerializeScene(activeSceneRef, editorScenePath);
        else
            SaveSceneAs();
    }

    void EditorLayer::SerializeScene(Ref<Scene> sceneRef, const std::filesystem::path& path)
    {
        SceneSerializer serializer(sceneRef);
        serializer.Serialize(path.string());
    }

    void EditorLayer::OnScenePlay()
    {
        m_SceneState = SceneState::Runtime;
        
        activeSceneRef = Scene::Copy(editorSceneRef);
        activeSceneRef->OnRuntimeStart();
        
        sceneHierarchyPanelRef.SetContext(activeSceneRef);
    }

    void EditorLayer::OnSceneStop() 
    {
        m_SceneState = SceneState::Editor;
        
        activeSceneRef->OnRuntimeStop();
        activeSceneRef = editorSceneRef;
        
        sceneHierarchyPanelRef.SetContext(activeSceneRef);
    }

    void EditorLayer::OnDuplicateEntity()
    {
        if (m_SceneState != SceneState::Editor)
            return;
        if (sceneHierarchyPanelRef.GetSelectedEntity())
            activeSceneRef->DuplicateEntity(sceneHierarchyPanelRef.GetSelectedEntity());
    }

    void EditorLayer::UIToolbar()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        auto& colors = ImGui::GetStyle().Colors;
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        float size = ImGui::GetWindowHeight() - 4.0f;
        Ref<Texture2D> icon = m_SceneState == SceneState::Editor ? iconPlayRef : iconStopRef;
        //ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
        ImGui::SetCursorPosX(.0f);
        if (ImGui::ImageButton("##playandstop",(ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1)))
        {
            if (m_SceneState == SceneState::Editor)
                OnScenePlay();
            else if (m_SceneState == SceneState::Runtime)
                OnSceneStop();
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        ImGui::End();
    }
}
