
#include "GameLayer.h"

#include "HRealEngine/Core/Application.h"
#include "HRealEngine/Core/KeyCodes.h"
#include "HRealEngine/Core/Logger.h"
#include "HRealEngine/Renderer/RenderCommand.h"
#include "HRealEngine/Scene/SceneSerializer.h"
#include "imgui/imgui.h"

namespace HRealEngine
{
    static const char* s_GameScenePath = "assets/scenes/GameScene.hrs";
    GameLayer::GameLayer() : Layer("GameLayer")
    {
    }

    void GameLayer::OnAttach()
    {
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);
        
        m_ActiveGameScene = CreateRef<Scene>();

        SceneSerializer serializer(m_ActiveGameScene);
        if (!serializer.Deserialize(s_GameScenePath))
            LOG_CORE_WARN("Failed to load scene from path: {0}", s_GameScenePath);
        m_ActiveGameScene->OnRuntimeStart();
    }

    void GameLayer::OnDetach()
    {
        if (m_ActiveGameScene)
            m_ActiveGameScene->OnRuntimeStop();
    }

    void GameLayer::OnUpdate(Timestep timestep)
    {
        m_Framebuffer->Resize(1280, 720);
        m_ActiveGameScene->OnViewportResize(1280, 720);

        m_Framebuffer->Bind();
        RenderCommand::Clear();
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        m_Framebuffer->ClearAttachment(1, -1);
        if (m_ActiveGameScene)
            m_ActiveGameScene->OnUpdateRuntime(timestep);
        m_Framebuffer->Unbind();
    }

    void GameLayer::OnImGuiRender()
    {
        ImGui::Begin("Game View");

        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)textureID, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
        
        ImGui::End();
    }

    void GameLayer::OnEvent(EventBase& eventRef)
    {
        EventDispatcher dispatcher(eventRef);
        dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(GameLayer::OnKeyPressed));
    }

    bool GameLayer::OnKeyPressed(KeyPressedEvent& event)
    {
        if (event.IsRepeating())
            return false;
        if (event.GetKeyCode() == HR_KEY_SPACE)
            LOG_CORE_INFO("Space key pressed in GameLayer");
        if (event.GetKeyCode() == HR_KEY_ESCAPE)
            Application::Get().Close();
        return false;
    }
}
