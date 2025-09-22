

#pragma once
#include "HRealEngine/Core/Core.h"
#include "HRealEngine/Core/Entity.h"

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
        
        template<typename Component>
        void ShowAddComponentEntry(const std::string& name);
        
        Ref<Scene> m_Context;
        Entity m_SelectedEntity;
    };
}
 