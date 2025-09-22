

#pragma once

namespace HRealEngine
{
    class GraphicsContext
    {
    public:
        virtual ~GraphicsContext() = default;
        virtual void SwapBuffers() = 0;
        virtual void Init() = 0;
    };
}