
#include "HRpch.h"
#include "RuntimeLayer.h"

#include "HRealEngine/Asset/AssetManager.h"
#include "HRealEngine/Core/Application.h"
#include "HRealEngine/Core/Input.h"
#include "HRealEngine/Events/KeyEvent.h"
#include "HRealEngine/Project/Project.h"
#include "HRealEngine/Renderer/RenderCommand.h"
#include "HRealEngine/Scripting/CSharpNodeRegistry.h"
#include "HRealEngine/Scripting/ScriptEngine.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace HRealEngine
{
    RuntimeLayer::RuntimeLayer() : Layer("RuntimeLayer")
    {
    }
    
    void RuntimeLayer::RegisterBehaviorTreeStufs()
    {
        if (ScriptEngine::IsInitialized())
        {
            auto& btActionClasses = ScriptEngine::s_BTActionClasses;
            for (const auto& [className, info] : btActionClasses)
            {
                CSharpNodeRegistry::AddManagedActionNode(className);
                LOG_CORE_INFO("Registered managed BT Action in editor: {}", className);
            }
        
            auto& btConditionClasses = ScriptEngine::s_BTConditionClasses;
            for (const auto& [className, info] : btConditionClasses)
            {
                CSharpNodeRegistry::AddManagedConditionNode(className);
                LOG_CORE_INFO("Registered managed BT Condition in editor: {}", className);
            }
        
            auto& btDecoratorClasses = ScriptEngine::s_BTDecoratorClasses;
            for (const auto& [className, info] : btDecoratorClasses)
            {
                CSharpNodeRegistry::AddManagedDecoratorNode(className);
                LOG_CORE_INFO("Registered managed BT Decorator in editor: {}", className);
            }
        
            auto& btBlackboardClasses = ScriptEngine::s_BTBlackboardClasses;
            for (const auto& [className, info] : btBlackboardClasses)
            {
                CSharpNodeRegistry::AddManagedBlackboard(className);
                LOG_CORE_INFO("Registered managed BT Blackboard in editor: {}", className);
            }
        }
    }
    
    void RuntimeLayer::OnAttach()
    {
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        if (!Project::GetActive())
        {
            LOG_CORE_ERROR("[Runtime] No project loaded. Pass a .hrpj as argv[1].");
            m_ProjectLoaded = false;
            return;
        }
        m_ProjectLoaded = true;
        RegisterBehaviorTreeStufs();
        AssetHandle startScene = Project::GetActive()->GetConfig().StartScene;
        if (!startScene)
        {
            LOG_CORE_ERROR("[Runtime] Project has no StartScene set!");
            return;
        }

        m_ActiveScene = AssetManager::GetAsset<Scene>(startScene);
        if (!m_ActiveScene)
        {
            LOG_CORE_ERROR("[Runtime] Failed to load StartScene asset!");
            return;
        }
        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        m_ActiveScene->Set2DPhysicsEnabled(false);
        m_ActiveScene->CreatePhysicsWorld();
        m_ActiveScene->OnRuntimeStart();

        LOG_CORE_INFO("[Runtime] Started project '{}' with StartScene handle {}",
            Project::GetActive()->GetConfig().Name, (uint64_t)startScene);

        Application::Get().CreateGameModeData();

    }

    void RuntimeLayer::OnDetach()
    {
        if (m_ActiveScene)
            m_ActiveScene->OnRuntimeStop();
        Application::Get().DestroyGameModeData();
    }

    void RuntimeLayer::OnUpdate(Timestep ts)
    {
        if (!m_ProjectLoaded || !m_ActiveScene)
            return;

        if (m_PendingSceneHandle != 0)
        {
            AssetHandle handle = m_PendingSceneHandle;
            m_PendingSceneHandle = 0;
            m_ActiveScene->OnRuntimeStop();
                
            //Ref<Scene> nextSceneAsset = AssetManager::GetAsset<Scene>(handle);
            auto eam = Project::GetActive()->GetEditorAssetManager();
            eam->ReloadAsset(handle);
            Ref<Scene> nextSceneAsset = AssetManager::GetAsset<Scene>(handle);
            if (nextSceneAsset)
            {
                m_ActiveScene = Scene::Copy(nextSceneAsset);
                m_ActiveScene->Set2DPhysicsEnabled(false);
                m_ActiveScene->CreatePhysicsWorld();
                m_ActiveScene->OnRuntimeStart();
                LOG_CORE_INFO("Runtime scene transitioned to new instance.");
            }
            return;
        }
        
        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
            m_ViewportSize.x > 0 && m_ViewportSize.y > 0 &&
            (spec.Width != (uint32_t)m_ViewportSize.x || spec.Height != (uint32_t)m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        m_Framebuffer->Bind();
        RenderCommand::Clear();
        m_Framebuffer->ClearAttachment(1, -1);

        m_ActiveScene->OnUpdateRuntime(ts);

        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];

        int mouseX = (int)mx;
        int mouseY = (int)(viewportSize.y - my);
        Input::SetViewportMousePos(mx, my);
        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
        {
            int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
            m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
            //LOG_CORE_INFO("Pixel data = {0}", pixelData);
            if (pixelData == -1)
                Input::SetHoveredEntity(nullptr);
            else
                Input::SetHoveredEntity(&m_HoveredEntity);
        }

        m_Framebuffer->Unbind();
    }


    void RuntimeLayer::OnImGuiRender()
    {
        const ImGuiViewport* vp = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(vp->Pos);
        ImGui::SetNextWindowSize(vp->Size);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::Begin("RuntimeViewport", nullptr, flags);
        
        m_ViewportBounds[0] = { vp->Pos.x, vp->Pos.y };
        m_ViewportBounds[1] = { vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y };

        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x > 0 && size.y > 0)
            m_ViewportSize = { size.x, size.y };

        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)(intptr_t)textureID, size, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();

        ImGui::PopStyleVar(3);
    }

    void RuntimeLayer::OnEvent(EventBase& eventRef)
    {
        EventDispatcher dispatcher(eventRef);
        dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& e) {
            if (e.GetKeyCode() == HR_KEY_ESCAPE)
                Application::Get().Close();
            return false;
        });
        dispatcher.Dispatch<SceneChangeEvent>(BIND_EVENT_FN(RuntimeLayer::OnSceneChange));
    }
    
    bool RuntimeLayer::OnSceneChange(SceneChangeEvent& event)
    {
        auto handle = event.GetSceneHandle();
        LOG_CORE_INFO("Scene change event received. New scene handle: {0}", handle);
        m_PendingSceneHandle = handle;
        return true;
    }
}
