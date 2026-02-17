#include "HRpch.h"
#include "BTEditorRoot.h"
#include "BTEditorApp.h"

namespace HRealEngine
{
    std::unique_ptr<BTEditorApp> BTEditorRoot::s_EditorApp;

    void BTEditorRoot::Start()
    {
        s_EditorApp = std::make_unique<BTEditorApp>();
        if (s_EditorApp)
            s_EditorApp->OnStart();
    }

    void BTEditorRoot::Tick()
    {
        if (s_EditorApp)
            s_EditorApp->Update();
    }

    void BTEditorRoot::Stop()
    {
        if (s_EditorApp)
            s_EditorApp.reset();
    }
}