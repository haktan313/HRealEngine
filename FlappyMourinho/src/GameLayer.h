#pragma once
#include "HRealEngine/Camera/SceneCamera.h"
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
        bool OnKeyPressed(KeyPressedEvent& event);
        
        Ref<Scene> m_ActiveGameScene;
        Ref<Framebuffer> m_Framebuffer;
        SceneCamera m_Camera;
        
    };
}
