#pragma once
#include <memory>

class NodeEditorApp;

class EditorRoot
{
public:
    static void EditorRootStart();
    static void EditorRootTick();
    static void EditorRootStop();

    static NodeEditorApp* GetNodeEditorApp() { return m_NodeEditorApp.get(); }
    static bool HasNodeEditorApp() { return m_NodeEditorApp != nullptr; }
private:
    static std::unique_ptr<NodeEditorApp> m_NodeEditorApp;
};
