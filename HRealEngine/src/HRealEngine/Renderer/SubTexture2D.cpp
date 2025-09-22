
#include "HRpch.h"
#include "SubTexture2D.h"

namespace HRealEngine
{
    SubTexture2D::SubTexture2D(const Ref<Texture2D>& textureRef, const glm::vec2& min, const glm::vec2& max) : m_textureRef(textureRef)
    {
        m_texCoords[0] = {min.x, min.y};
        m_texCoords[1] = {max.x, min.y};
        m_texCoords[2] = {max.x, max.y};
        m_texCoords[3] = {min.x, max.y};
    }

    Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& textureRef, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize)
    {
        glm::vec2 min = { (coords.x * cellSize.x) / textureRef->GetWidth(), (coords.y * cellSize.y) / textureRef->GetHeight() };
        glm::vec2 max = { ((coords.x + spriteSize.x) * cellSize.x) / textureRef->GetWidth(), ((coords.y + spriteSize.y) * cellSize.y) / textureRef->GetHeight() };
        return CreateRef<SubTexture2D>(textureRef, min, max);
    }
}
