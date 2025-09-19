
//SceneHierarchyPanel.h
#pragma once
#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Scene/Scene.h"
#include "HRealEngine/Scene/Entity.h"

namespace HRealEngine
{
    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Ref<Scene>& context);

        void SetContext(const Ref<Scene>& context);
        void OnImGuiRender();
        Entity GetSelectedEntity() const { return m_SelectedEntity; }
        void SetSelectedEntity(Entity entity);
    private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);
        
        Ref<Scene> m_Context;
        Entity m_SelectedEntity;
    };
}
 