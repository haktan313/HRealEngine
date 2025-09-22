

#pragma once
#include "ParticleSystem.h"
#include "HRealEngine/Core/Layer.h"
#include "HRealEngine/Events/KeyEvent.h"
#include "HRealEngine/Renderer/Framebuffer.h"
#include "HRealEngine/Camera/OrthCameraController.h"
#include "HRealEngine/Renderer/SubTexture2D.h"
#include "HRealEngine/Scene/Scene.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/SceneHierarchyPanel.h"

namespace HRealEngine
{
    class EditorLayer : public HRealEngine::Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;
        
        void OnAttach() override;
        void OnDetach() override;
        
        void OnUpdate(Timestep timestep) override;
        void OnImGuiRender() override;
        void OnEvent(EventBase& eventRef) override;
    private:
        bool OnKeyPressed(KeyPressedEvent& event);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& event);

        void OnOverlayRender();

        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveSceneAs();
        void SaveScene();

        void SerializeScene(Ref<Scene> sceneRef, const std::filesystem::path& path);

        void OnScenePlay();
        void OnSceneSimulate();
        void OnSceneStop();

        void OnDuplicateEntity();

        void UIToolbar();

        enum class SceneState
        {
            Editor = 0,
            Runtime = 1,
            Simulate = 2
        };
        SceneState m_SceneState = SceneState::Editor;
        
        OrthCameraController m_OrthCameraController;
        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_ActiveScene;
        Ref<Scene> m_EditorScene;
        std::filesystem::path m_EditorScenePath;
        int m_GizmoType = -1; // -1: none, 0: translate, 1: rotate, 2: scale
        SceneHierarchyPanel m_SceneHierarchyPanel;
        ContentBrowserPanel m_ContentBrowserPanel;

        Entity m_CameraEntity;
        Entity m_HoveredEntity;
        EditorCamera m_EditorCamera;

        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
        glm::vec2 m_ViewportBounds[2];
        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        bool m_ShowPhysicsColliders = false;

        Ref<Texture2D> m_JoseMourinhoTexture;
        Ref<Texture2D> m_CheckBoardTexture;
        Ref<Texture2D> m_SpriteSheet;

        Ref<Texture2D> m_IconPlay;
        Ref<Texture2D> m_IconStop;
        Ref<Texture2D> m_IconSimulate;
        
        Ref<SubTexture2D> m_TextureStairs;
        Ref<SubTexture2D> m_TextureTree;
        Ref<SubTexture2D> m_TextureBarrel;

        struct ProfileResult
        {
            const char* Name;
            float Time;
        };
        std::vector<ProfileResult> m_ProfileResults;

        ParticleSystem m_ParticleSystem;
        ParticleProps m_Particle;
    };
}
