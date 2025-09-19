
//OrthCameraController.h
#pragma once
#include "OrthCamera.h"
#include "HRealEngine/Core/Timestep.h"
#include "HRealEngine/Events/AppEvent.h"
#include "HRealEngine/Events/EventBase.h"
#include "HRealEngine/Events/MouseEvent.h"

namespace HRealEngine
{
    struct OrthCameraBounds
    {
        float Left, Right;
        float Bottom, Top;

        float GetWidth() const { return Right - Left; }
        float GetHeight() const { return Top - Bottom; }
    };
    class OrthCameraController
    {
    public:
        OrthCameraController(float aspectRatio, bool rotation = false);

        void OnUpdate(Timestep timestep);
        void OnEvent(EventBase& eventRef);

        void OnResize(float width, float height);

        OrthCamera& GetCamera() { return orthCameraRef; }
        const OrthCamera& GetCamera() const { return orthCameraRef; }

        float GetZoomLevel() const { return zoomLevelRef; }
        void SetZoomLevel(float level) { zoomLevelRef = level; CalculateView(); }

        const OrthCameraBounds& GetBounds() const { return cameraBoundsRef; }
    private:
        void CalculateView();
        bool OnMouseScrolled(MouseScrolledEvent& eventRef);
        bool OnWindowResized(WindowResizeEvent& eventRef);
        
        float aspectRatioRef;
        float zoomLevelRef = 1.0f;
        bool bRotationRef;
        OrthCameraBounds cameraBoundsRef;
        OrthCamera orthCameraRef;

        glm::vec3 cameraPosition = { 0.0f, 0.0f, 0.0f };
        float cameraMoveSpeed = 2.f, cameraRotationSpeed = 90.0f;
        float cameraRotation = 0.0f;
    };
}
