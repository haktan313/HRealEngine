
//Shader.cpp
#include "HRpch.h"
#include "Shader.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace HRealEngine
{
	Ref<Shader> Shader::Create(const std::string& filePath)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			HREALENGINE_CORE_DEBUGBREAK(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLShader>(filePath);
		}
		HREALENGINE_CORE_DEBUGBREAK(false, "Unknown RendererAPI!");
		return nullptr;
	}

    Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource)
    {
	    switch (Renderer::GetAPI())
	    {
		    case RendererAPI::API::None:
			    HREALENGINE_CORE_DEBUGBREAK(false, "RendererAPI::None is not supported!");
			    return nullptr;
	    	case RendererAPI::API::OpenGL:
			    return CreateRef<OpenGLShader>(name, vertexSource, fragmentSource);
	    }
    	HREALENGINE_CORE_DEBUGBREAK(false, "Unknown RendererAPI!");
    	return nullptr;
    }

	//---------------------------------------------------------------

    void ShaderLibrary::Add(const Ref<Shader>& shader)
    {
		auto& name = shader->GetName();
		HREALENGINE_CORE_DEBUGBREAK(!Exists(name), "Shader already exists!");
		shaders[name] = shader;
    }

    void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
    {
		shaders[name] = shader;
    }

    Ref<Shader> ShaderLibrary::Load(const std::string& filePath)
    {
		auto shader = Shader::Create(filePath);
		Add(shader);
		return shader;
    }

    Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filePath)
    {
		auto shader = Shader::Create(filePath);
		Add(name, shader);
		return shader;
    }

    Ref<Shader> ShaderLibrary::Get(const std::string& name)
    {
		HREALENGINE_CORE_DEBUGBREAK(Exists(name), "Shader not found!");
		return shaders[name];
    }

    bool ShaderLibrary::Exists(const std::string& name) const
    {
		return shaders.find(name) != shaders.end();
    }
}
