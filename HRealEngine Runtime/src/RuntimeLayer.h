#pragma once
#include "glm/vec2.hpp"
#include "HRealEngine/Core/Entity.h"
#include "HRealEngine/Core/Layer.h"
#include "HRealEngine/Renderer/Framebuffer.h"
#include "HRealEngine/Scene/Scene.h"

namespace HRealEngine
{
    class RuntimeLayer : public Layer
    {
    public:
        RuntimeLayer();
        virtual ~RuntimeLayer() = default;

        void OnAttach() override;
        void OnDetach() override;
        
        void OnUpdate(Timestep timestep) override;
        void OnImGuiRender() override;
        void OnEvent(EventBase& eventRef) override;
        void Dockspace();

        bool OnSceneChange(SceneChangeEvent& event);

    private:
        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_ActiveScene;
        Entity m_HoveredEntity;
        AssetHandle m_PendingSceneHandle = 0;
        
        glm::vec2 m_ViewportSize = {1920.f, 1080.f};
        glm::vec2 m_ViewportBounds[2];
        
        bool m_ProjectLoaded = false; 
    };
}
