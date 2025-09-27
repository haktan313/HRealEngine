
#include "GameLayer.h"
#include <filesystem>
#include "aliPipe.h"
#include "MourinhoController.h"
#include "Box2D/include/box2d/b2_fixture.h"
#include "Box2D/include/box2d/b2_polygon_shape.h"
#include "Box2D/include/box2d/b2_world.h"
#include "HRealEngine/Core/Application.h"
#include "HRealEngine/Core/Components.h"
#include "HRealEngine/Core/Entity.h"
#include "HRealEngine/Core/KeyCodes.h"
#include "HRealEngine/Core/Logger.h"
#include "HRealEngine/Renderer/RenderCommand.h"
#include "HRealEngine/Scene/SceneSerializer.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace HRealEngine
{
    static const char* s_GameScenePath = "assets/scenes/GameScene.hrs";
    extern const std::filesystem::path g_AssetsDirectory = "assets";
    
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

        auto& registery = m_ActiveGameScene->GetRegistry();
        for (auto& entity : registery.view<TagComponent>())
        {
            auto& tag = registery.get<TagComponent>(entity);
            if (tag.Tag == "bird")
            {
                auto ent = Entity{entity, m_ActiveGameScene.get()};
                ent.AddComponent<NativeScriptComponent>().Bind<MourinhoController>();
            }
            if (tag.Tag == "aliTop")
            {
                auto ent = Entity{entity,m_ActiveGameScene.get()};
                ent.AddComponent<NativeScriptComponent>().Bind<aliPipe>();
                m_AliTop = ent;
            }
            if (tag.Tag == "aliBottom")
            {
                auto ent = Entity{entity,m_ActiveGameScene.get()};
                ent.AddComponent<NativeScriptComponent>().Bind<aliPipe>();
                m_AliBottom = ent;
            }
        }
        
        m_ActiveGameScene->OnRuntimeStart();
    }

    void GameLayer::OnDetach()
    {
        if (m_ActiveGameScene)
            m_ActiveGameScene->OnRuntimeStop();
    }

    void GameLayer::OnUpdate(Timestep timestep)
    {
        m_SpawnTimer += timestep;
        if (m_SpawnTimer >= m_SpawnInterval)
            SpawnNewAliPipe();
        
        m_Framebuffer->Resize(1280, 720);
        m_ActiveGameScene->OnViewportResize(1280, 720);

        m_Framebuffer->Bind();
        RenderCommand::Clear();
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        m_Framebuffer->ClearAttachment(1, -1);
        if (m_ActiveGameScene)
            m_ActiveGameScene->OnUpdateRuntime(timestep);
        m_Framebuffer->Unbind();

        int amountOfEntities = m_ActiveGameScene->GetRegistry().view<TagComponent>().size();
        LOG_CORE_INFO("Entities in scene: {0}", amountOfEntities);
    }

    void GameLayer::OnImGuiRender()
    {
        Dockspace();
        ImGui::Begin("Game View");
        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)(intptr_t)textureID, ImGui::GetContentRegionAvail(), ImVec2(0,1), ImVec2(1,0));
        ImGui::End();
    }

    void GameLayer::OnEvent(EventBase& eventRef)
    {
        EventDispatcher dispatcher(eventRef);
        dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& e) {
            if (e.GetKeyCode() == HR_KEY_ESCAPE)
                Application::Get().Close();
            return false;
        });
    }

    void GameLayer::SpawnNewAliPipe()
    {
        m_SpawnTimer = 0.0f;
        
        Entity eBottom = m_ActiveGameScene->CreateEntity(m_AliBottom.GetName());
        
        //eBottom.AddOrReplaceComponent<TransformComponent>(m_AliBottom.GetComponent<TransformComponent>());
        auto& tcBottom = m_AliBottom.GetComponent<TransformComponent>();
        auto& tcNewBottom = eBottom.AddOrReplaceComponent<TransformComponent>();
        tcNewBottom.Position = tcBottom.Position + glm::vec3(1.6f,0.0f,0.0f);
        tcNewBottom.Rotation = tcBottom.Rotation;
        tcNewBottom.Scale = tcBottom.Scale;

        if (m_AliBottom.HasComponent<SpriteRendererComponent>())
            eBottom.AddOrReplaceComponent<SpriteRendererComponent>(m_AliBottom.GetComponent<SpriteRendererComponent>());
        if (m_AliBottom.HasComponent<Rigidbody2DComponent>())
            eBottom.AddComponent<Rigidbody2DComponent>();
        if (m_AliBottom.HasComponent<BoxCollider2DComponent>())
            eBottom.AddComponent<BoxCollider2DComponent>();
        {
            auto& tc  = eBottom.GetComponent<TransformComponent>();
            auto& rb = eBottom.GetComponent<Rigidbody2DComponent>();

            b2BodyDef def;
            def.type = b2_kinematicBody;
            def.position.Set(tc.Position.x, tc.Position.y);
            def.angle = tc.Rotation.z;

            b2Body* body = m_ActiveGameScene->GetPhysicsWorld()->CreateBody(&def);
            body->SetFixedRotation(rb.FixedRotation);
            rb.RuntimeBody = body;

            if (eBottom.HasComponent<BoxCollider2DComponent>())
            {
                auto& bc = eBottom.GetComponent<BoxCollider2DComponent>();
                b2PolygonShape shape;
                shape.SetAsBox(bc.Size.x * tc.Scale.x, bc.Size.y * tc.Scale.y);

                b2FixtureDef fd;
                fd.shape = &shape;
                fd.density = bc.Density;
                fd.friction = bc.Friction;
                fd.restitution = bc.Restitution;
                fd.restitutionThreshold = bc.RestitutionThreshold;
                body->CreateFixture(&fd);
            }
        }
        eBottom.AddOrReplaceComponent<NativeScriptComponent>().Bind<aliPipe>();
        m_AliBottom = eBottom;
        
        Entity eTop = m_ActiveGameScene->CreateEntity(m_AliTop.GetName());

        //eTop.AddOrReplaceComponent<TransformComponent>(m_AliTop.GetComponent<TransformComponent>());
        auto& tcTop = m_AliTop.GetComponent<TransformComponent>();
        auto& tcNewTop = eTop.AddOrReplaceComponent<TransformComponent>();
        tcNewTop.Position = tcTop.Position + glm::vec3(1.6f,0.0f,0.0f);
        tcNewTop.Rotation = tcTop.Rotation;
        tcNewTop.Scale = tcTop.Scale;
         
        if (m_AliTop.HasComponent<SpriteRendererComponent>())
            eTop.AddOrReplaceComponent<SpriteRendererComponent>(m_AliTop.GetComponent<SpriteRendererComponent>());
        if (m_AliTop.HasComponent<Rigidbody2DComponent>())
            eTop.AddComponent<Rigidbody2DComponent>();
        if (m_AliTop.HasComponent<BoxCollider2DComponent>())
            eTop.AddComponent<BoxCollider2DComponent>();
        {
            auto& tc  = eTop.GetComponent<TransformComponent>();
            auto& rb = eTop.GetComponent<Rigidbody2DComponent>();

            b2BodyDef def;
            def.type = b2_kinematicBody;
            def.position.Set(tc.Position.x, tc.Position.y);
            def.angle = tc.Rotation.z;

            b2Body* body = m_ActiveGameScene->GetPhysicsWorld()->CreateBody(&def);
            body->SetFixedRotation(rb.FixedRotation);
            rb.RuntimeBody = body;

            if (eTop.HasComponent<BoxCollider2DComponent>())
            {
                auto& bc = eTop.GetComponent<BoxCollider2DComponent>();
                b2PolygonShape shape;
                shape.SetAsBox(bc.Size.x * tc.Scale.x, bc.Size.y * tc.Scale.y);

                b2FixtureDef fd;
                fd.shape = &shape;
                fd.density = bc.Density;
                fd.friction = bc.Friction;
                fd.restitution = bc.Restitution;
                fd.restitutionThreshold = bc.RestitutionThreshold;
                body->CreateFixture(&fd);
            }
        }
        eTop.AddOrReplaceComponent<NativeScriptComponent>().Bind<aliPipe>();
        m_AliTop = eTop;
    }

    void GameLayer::Dockspace()
    {
        static bool dockspaceOpen = true;
        static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                       ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_NoBringToFrontOnFocus | 
                       ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("DockSpaceRoot", &dockspaceOpen, windowFlags);
        ImGui::PopStyleVar(2);
        
        ImGuiID dockspaceID = ImGui::GetID("GameDockSpace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

        static bool first_time = true;
        if (first_time)
        {
            first_time = false;
            ImGui::DockBuilderRemoveNode(dockspaceID);
            ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderDockWindow("Game View", dockspaceID);
            ImGui::DockBuilderFinish(dockspaceID);
        }
        ImGui::End();
    }
}
