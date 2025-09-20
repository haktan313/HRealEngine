
//EditorLayer.h
#pragma once
#include "ParticleSystem.h"
#include "HRealEngine/Core/Layer.h"
#include "HRealEngine/Events/KeyEvent.h"
#include "HRealEngine/Renderer/Framebuffer.h"
#include "HRealEngine/Renderer/OrthCameraController.h"
#include "HRealEngine/Renderer/Texture.h"
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
        void OnSceneStop();

        void OnDuplicateEntity();

        void UIToolbar();

        enum class SceneState
        {
            Editor = 0,
            Runtime = 1
        };
        SceneState m_SceneState = SceneState::Editor;
        
        OrthCameraController orthCameraControllerRef;
        Ref<Framebuffer> framebufferRef;
        Ref<Scene> activeSceneRef;
        Ref<Scene> editorSceneRef;
        std::filesystem::path editorScenePath;
        int m_GizmoType = -1; // -1: none, 0: translate, 1: rotate, 2: scale
        SceneHierarchyPanel sceneHierarchyPanelRef;
        ContentBrowserPanel contentBrowserPanelRef;

        Entity cameraEntity;
        Entity m_HoveredEntity;
        EditorCamera m_EditorCamera;

        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
        glm::vec2 m_ViewportBounds[2];
        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        bool m_ShowPhysicsColliders = false;

        Ref<Texture2D> joseMourinhoTextureRef;
        Ref<Texture2D> checkBoardTextureRef;
        Ref<Texture2D> spriteSheetRef;

        Ref<Texture2D> iconPlayRef;
        Ref<Texture2D> iconStopRef;
        
        Ref<SubTexture2D> textureStairs;
        Ref<SubTexture2D> textureTree;
        Ref<SubTexture2D> textureBarrel;

        struct ProfileResult
        {
            const char* Name;
            float Time;
        };
        std::vector<ProfileResult> profileResults;

        ParticleSystem m_ParticleSystem;
        ParticleProps m_Particle;
    };
}
