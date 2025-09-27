#pragma once
#include "HRealEngine/Core/Entity.h"
#include "HRealEngine/Core/Layer.h"
#include "HRealEngine/Events/KeyEvent.h"
#include "HRealEngine/Renderer/Framebuffer.h"
#include "HRealEngine/Scene/Scene.h"

namespace HRealEngine
{
    class GameLayer : public Layer
    {
    public:
        GameLayer();
        virtual ~GameLayer() = default;
        
        void OnAttach() override;
        void OnDetach() override;
        
        void OnUpdate(Timestep timestep) override;
        void OnImGuiRender() override;
        void OnEvent(EventBase& eventRef) override;
    private:
        void SpawnNewAliPipe();
        void Dockspace();

        Entity m_AliTop, m_AliBottom; 
        Ref<Scene> m_ActiveGameScene;
        Ref<Framebuffer> m_Framebuffer;
        float m_SpawnTimer = 0.0f;
        float  m_SpawnInterval = 1.5f;
    };
}
