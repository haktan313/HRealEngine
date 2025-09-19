
//Sandbox2D.cpp
#include "Sandbox2D.h"
#include "glm/gtc/type_ptr.hpp"
#include <chrono>
#include "HRealEngine/Renderer/RenderCommand.h"
#include "HRealEngine/Renderer/Renderer2D.h"
#include "imgui/imgui.h"


Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), orthCameraControllerRef(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
    joseMourinhoTextureRef = HRealEngine::Texture2D::Create("assets/textures/joseMourinho.png");
    checkBoardTextureRef = HRealEngine::Texture2D::Create("assets/textures/Checkerboard.png");
    spriteSheetRef = HRealEngine::Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

    textureStairs = HRealEngine::SubTexture2D::CreateFromCoords(spriteSheetRef, {7, 6}, {128, 128});
    textureTree = HRealEngine::SubTexture2D::CreateFromCoords(spriteSheetRef, {2, 1}, {128, 128}, {1,2});
    textureBarrel = HRealEngine::SubTexture2D::CreateFromCoords(spriteSheetRef, {8, 2}, {128, 128});
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(HRealEngine::Timestep timestep)
{
    HR_PROFILE_FUNC();
    
    {
        HR_PROFILE_SCOPE("CameraController::OnUpdate");
        orthCameraControllerRef.OnUpdate(timestep);
    }
    HRealEngine::Renderer2D::ResetStats();
    {
        HR_PROFILE_SCOPE("Renderer Prep");
        HRealEngine::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        HRealEngine::RenderCommand::Clear();
    }

    {
        HR_PROFILE_SCOPE("Renderer Draw");
        HRealEngine::Renderer2D::BeginScene(orthCameraControllerRef.GetCamera());

        HRealEngine::Renderer2D::DrawRotatedQuad({-0.2f, 0.5f}, {0.8f, 0.8f}, glm::radians(-75.f), {0.8f, 0.2f, 0.3f, 1.0f});
        HRealEngine::Renderer2D::DrawQuad({-0.7f, 0.0f}, {0.8f, 0.4f}, {0.2f, 0.8f, 0.3f, 1.0f});
        HRealEngine::Renderer2D::DrawQuad({0.5f, -0.5f}, {1.f, 1.0f}, joseMourinhoTextureRef);
        HRealEngine::Renderer2D::DrawRotatedQuad({0.f, 0.f}, {1.f, 1.f}, glm::radians(45.f), checkBoardTextureRef, 30.f, glm::vec4(.5f, 0.3f, 0.9f, 1.0f));
        HRealEngine::Renderer2D::DrawQuad({0.0f, 0.0f}, {10.f, 10.f}, checkBoardTextureRef, 10.f);
    
        HRealEngine::Renderer2D::EndScene();
    }
    
    HRealEngine::Renderer2D::BeginScene(orthCameraControllerRef.GetCamera());
    HRealEngine::Renderer2D::DrawQuad({0.f,0.f, 0.5f}, {1.f,1.f}, textureStairs);
    HRealEngine::Renderer2D::DrawQuad({-1.f,0.f, 0.5f}, {1.f,2.f}, textureTree);
    HRealEngine::Renderer2D::DrawQuad({1.f,0.f, 0.5f}, {1.f,1.f}, textureBarrel);
    HRealEngine::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
    ImGui::Begin("Profile Results");
    auto stats = HRealEngine::Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("Quads: %d", stats.QuadCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
    ImGui::End();
}

void Sandbox2D::OnEvent(HRealEngine::EventBase& eventRef)
{
    orthCameraControllerRef.OnEvent(eventRef);
}
