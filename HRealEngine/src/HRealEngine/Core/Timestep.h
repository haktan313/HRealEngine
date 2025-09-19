
//Timestep.h
#pragma once

namespace HRealEngine
{
    class Timestep
    {
    public:
        Timestep(float time = 0.0f)   
            : timeRef(time)
        {
        }
        operator float() const { return timeRef; }
        
        float GetSeconds() const { return timeRef; }
        float GetMilliseconds() const { return timeRef * 1000.0f; }
    private:
        float timeRef;
    };
}
