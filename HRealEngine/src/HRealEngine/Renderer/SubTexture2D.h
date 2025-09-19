
//SubTexture2D.h
#pragma once
#include "Texture.h"

namespace HRealEngine
{
    class SubTexture2D
    {
    public:
        SubTexture2D(const Ref<Texture2D>& textureRef, const glm::vec2& min, const glm::vec2& max);

        const Ref<Texture2D>& GetTexture() const { return m_textureRef; }
        const glm::vec2* GetTexCoords() const { return m_texCoords; }

        static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& textureRef, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize = { 1,1 });
    private:
        Ref<Texture2D> m_textureRef;
        glm::vec2 m_texCoords[4];
    };
}
