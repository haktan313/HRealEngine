
//OpenGLShader.h
#pragma once
#include "HRealEngine/Renderer/Shader.h"
#include <glm/glm.hpp>

typedef unsigned int GLenum;

namespace HRealEngine
{
    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
        OpenGLShader(const std::string& filePath);
        virtual ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        void SetInt(const std::string& name, int value) override;
        void SetIntArray(const std::string& name, int* values, uint32_t count) override;
        void SetFloat(const std::string& name, float value) override;
        void SetFloat3(const std::string& name,const glm::vec3& value) override;
        void SetFloat4(const std::string& name,const glm::vec4& value) override;
        void SetMat4(const std::string& name,const glm::mat4& value) override;

        virtual const std::string& GetName() const override { return shaderName; }
        
        void UploadUniformFloat(const std::string& uniformName, float values);
        void UploadUniformFloat2(const std::string& uniformName, const glm::vec2& values);
        void UploadUniformFloat3(const std::string& uniformName, const glm::vec3& values);
        void UploadUniformFloat4(const std::string& uniformName, const glm::vec4& values);

        void UploadUniformInt(const std::string& uniformName, int value);
        void UploadUniformIntArray(const std::string& uniformName, int* values, uint32_t count);

        void UploadUniformMat3(const std::string& uniformName, const glm::mat3& matrix);
        void UploadUniformMat4(const std::string& uniformName, const glm::mat4& matrix);
    private:
        std::string ReadFile(const std::string& filePath);
        std::unordered_map<GLenum,std::string> PreProcess(const std::string& source);
        void Compile(const std::unordered_map<GLenum,std::string>& shaderSources);

        uint32_t rendererID;
        std::string shaderName;
    };
}
