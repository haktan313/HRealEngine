

//OrthCameraController.cpp
#include "HRpch.h"
#include "OrthCameraController.h"
#include "HRealEngine/Core/Input.h"
#include "HRealEngine/Core/KeyCodes.h"

namespace HRealEngine
{
    OrthCameraController::OrthCameraController(float aspectRatio, bool rotation)
        : aspectRatioRef(aspectRatio), bRotationRef(rotation), cameraBoundsRef({ -aspectRatio * zoomLevelRef, aspectRatio * zoomLevelRef, -zoomLevelRef, zoomLevelRef }), orthCameraRef(cameraBoundsRef.Left, cameraBoundsRef.Right, cameraBoundsRef.Bottom, cameraBoundsRef.Top)
            
    {
        
    }

    void OrthCameraController::OnUpdate(Timestep timestep)
    {
        if (Input::IsKeyPressed(HR_KEY_A))
            cameraPosition.x -= cameraMoveSpeed * timestep;
        else if (Input::IsKeyPressed(HR_KEY_D))
            cameraPosition.x += cameraMoveSpeed * timestep;
        if (Input::IsKeyPressed(HR_KEY_W))
            cameraPosition.y += cameraMoveSpeed * timestep;
        else if (Input::IsKeyPressed(HR_KEY_S))
            cameraPosition.y -= cameraMoveSpeed * timestep;

        if (bRotationRef)
        {
            if (Input::IsKeyPressed(HR_KEY_Q))
                cameraRotation -= cameraRotationSpeed * timestep;
            else if (Input::IsKeyPressed(HR_KEY_E))
                cameraRotation += cameraRotationSpeed * timestep;
            orthCameraRef.SetRotation(cameraRotation);
        }
        orthCameraRef.SetPosition(cameraPosition);
        cameraMoveSpeed = zoomLevelRef;
    }

    void OrthCameraController::OnEvent(EventBase& eventRef)
    {
        EventDispatcher dispatcher(eventRef);
        dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(OrthCameraController::OnMouseScrolled));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OrthCameraController::OnWindowResized));
    }

    void OrthCameraController::OnResize(float width, float height)
    {
        aspectRatioRef = width / height;
        CalculateView();
    }

    void OrthCameraController::CalculateView()
    {
        cameraBoundsRef = { -aspectRatioRef * zoomLevelRef, aspectRatioRef * zoomLevelRef, -zoomLevelRef, zoomLevelRef };
        orthCameraRef.SetProjection(cameraBoundsRef.Left, cameraBoundsRef.Right, cameraBoundsRef.Bottom, cameraBoundsRef.Top);
    }

    bool OrthCameraController::OnMouseScrolled(MouseScrolledEvent& eventRef)
    {
        zoomLevelRef -= eventRef.GetOffsetY() * 0.25f;
        zoomLevelRef = std::max(zoomLevelRef, 0.25f);
        CalculateView();
        return false;
    }

    bool OrthCameraController::OnWindowResized(WindowResizeEvent& eventRef)
    {
        OnResize((float)eventRef.GetWidth(), (float)eventRef.GetHeight());
        return false;
    }
}
