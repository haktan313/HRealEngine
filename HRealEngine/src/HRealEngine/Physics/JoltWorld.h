#pragma once
#include "JoltWorldHelper.h"

namespace HRealEngine
{
    class Scene;
    
    class JoltWorld
    {
    public:
        JoltWorld(Scene* scene);
        ~JoltWorld();
        void Init();
        
    private:
        Scene* m_Scene = nullptr;
        Scope<JoltWorldHelper> m_JoltWorldHelper;
    };
}
