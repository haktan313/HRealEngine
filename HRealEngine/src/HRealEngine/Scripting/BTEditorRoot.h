#pragma once
#include <memory>

namespace HRealEngine
{
    class BTEditorApp;

    class BTEditorRoot
    {
    public:
        static void Start();
        static void Tick();
        static void Stop();

        static BTEditorApp* GetEditorApp() { return s_EditorApp.get(); }
        static bool HasEditorApp() { return s_EditorApp != nullptr; }

    private:
        static std::unique_ptr<BTEditorApp> s_EditorApp;
    };
}