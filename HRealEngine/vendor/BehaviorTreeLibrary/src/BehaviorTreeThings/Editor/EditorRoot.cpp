#include "EditorRoot.h"
#include "NodeEditorApp.h"

std::unique_ptr<NodeEditorApp> EditorRoot::m_NodeEditorApp;

void EditorRoot::EditorRootStart()
{
    m_NodeEditorApp = std::make_unique<NodeEditorApp>();
    if (m_NodeEditorApp)
        m_NodeEditorApp->OnStart();
}

void EditorRoot::EditorRootTick()
{
    if (m_NodeEditorApp)
        m_NodeEditorApp->Update();
}

void EditorRoot::EditorRootStop()
{
    if (m_NodeEditorApp)
        m_NodeEditorApp = nullptr;
}
