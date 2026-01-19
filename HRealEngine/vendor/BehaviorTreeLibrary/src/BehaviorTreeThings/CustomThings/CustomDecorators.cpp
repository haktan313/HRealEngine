#include "CustomDecorators.h"

#include <iostream>
#include <GLFW/glfw3.h>

void ChangeResultOfTheNodeDecorator::OnStart()
{
    HDecorator::OnStart();
}

bool ChangeResultOfTheNodeDecorator::CanExecute()
{
    return true;
}

void ChangeResultOfTheNodeDecorator::OnFinishedResult(NodeStatus& status)
{
    status = m_NewResult;
}

void ChangeResultOfTheNodeDecorator::OnFinished()
{
    HDecorator::OnFinished();
}

void ChangeResultOfTheNodeDecorator::OnAbort()
{
    HDecorator::OnAbort();
}

void CooldownDecorator::OnStart()
{
    HDecorator::OnStart();
}

bool CooldownDecorator::CanExecute()
{
    auto currentTime = static_cast<float>(glfwGetTime());
    if (currentTime - m_LastExecutionTime <= m_CooldownTime)
    {
        std::cout << "CooldownDecorator: On cooldown. Cannot execute child node yet.\n";
        std::cout << "Time remaining: " << (m_CooldownTime - (currentTime - m_LastExecutionTime)) << " seconds.\n";
        return false;
    }
    std::cout << "CooldownDecorator: Cooldown complete. Can execute child node.\n";
    return true;
}

void CooldownDecorator::OnFinishedResult(NodeStatus& status)
{
    m_LastExecutionTime = static_cast<float>(glfwGetTime());
    std::cout << "CooldownDecorator: Child node executed. Starting cooldown of " << m_CooldownTime << " seconds.\n";
}

void CooldownDecorator::OnFinished()
{
    HDecorator::OnFinished();
}

void CooldownDecorator::OnAbort()
{
    HDecorator::OnAbort();
}
