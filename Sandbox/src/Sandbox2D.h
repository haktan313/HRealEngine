
//Sandbox2D.h
#pragma once
#include "HRealEngine/Core/Layer.h"
#include "HRealEngine/Renderer/OrthCameraController.h"
#include "HRealEngine/Renderer/Texture.h"
#include "HRealEngine/Renderer/VertexArray.h"
#include "HRealEngine/Renderer/SubTexture2D.h"

class Sandbox2D : public HRealEngine::Layer
{
public:
    Sandbox2D();
    virtual ~Sandbox2D() = default;
    
    void OnAttach() override;
    void OnDetach() override;
    
    void OnUpdate(HRealEngine::Timestep timestep) override;
    void OnImGuiRender() override;
    void OnEvent(HRealEngine::EventBase& eventRef) override;
private:
    HRealEngine::OrthCameraController orthCameraControllerRef;


    HRealEngine::Ref<HRealEngine::Texture2D> joseMourinhoTextureRef;
    HRealEngine::Ref<HRealEngine::Texture2D> checkBoardTextureRef;
    
    HRealEngine::Ref<HRealEngine::Texture2D> spriteSheetRef;
    HRealEngine::Ref<HRealEngine::SubTexture2D> textureStairs;
    HRealEngine::Ref<HRealEngine::SubTexture2D> textureTree;
    HRealEngine::Ref<HRealEngine::SubTexture2D> textureBarrel;

    struct ProfileResult
    {
        const char* Name;
        float Time;
    };
    std::vector<ProfileResult> profileResults;
};
