

#pragma once

namespace HRealEngine
{
    class Timestep
    {
    public:
        Timestep(float time = 0.0f)   
            : m_TimeRef(time)
        {
        }
        operator float() const { return m_TimeRef; }
        
        float GetSeconds() const { return m_TimeRef; }
        float GetMilliseconds() const { return m_TimeRef * 1000.0f; }
    private:
        float m_TimeRef;
    };
}
