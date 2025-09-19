
//OpenGLShader.cpp
#include "HRpch.h"
#include "OpenGLShader.h"
#include "glad/glad.h"
#include "fstream"
#include "glm/gtc/type_ptr.hpp"
#include "HRealEngine/Core/Core.h"

namespace HRealEngine
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;
		HREALENGINE_CORE_DEBUGBREAK(false, "Unknown shader type!");
		return 0;
	}

    OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource) : shaderName(name)
    {
		std::unordered_map<GLenum, std::string> shaderSources;
		shaderSources[GL_VERTEX_SHADER] = vertexSource;
		shaderSources[GL_FRAGMENT_SHADER] = fragmentSource;
		Compile(shaderSources);
    }

    OpenGLShader::OpenGLShader(const std::string& filePath)
    {
		std::string source = ReadFile(filePath);
		auto shaderSource = PreProcess(source);
		Compile(shaderSource);

		auto lastSlash = filePath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filePath.rfind('.');
		auto count = lastDot == std::string::npos ? filePath.size() - lastSlash : lastDot - lastSlash;
		shaderName = filePath.substr(lastSlash, count);
    }

	std::string OpenGLShader::ReadFile(const std::string& filePath)
	{
    	std::string result;
    	std::ifstream in(filePath, std::ios::in, std::ios::binary);
    	if (in)
    	{
    		in.seekg(0, std::ios::end);//Moves the file cursor to the end.
    		result.resize(in.tellg());//Resize the string so it has enough space to hold the whole file.
    		in.seekg(0, std::ios::beg);//Moves the cursor back to the start of the file.
    		in.read(&result[0], result.size());
    		in.close();
    	}
    	else
    	{
    		HREALENGINE_CORE_DEBUGBREAK(false, "Could not open file '{0}'", filePath);
    	}
    	return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
    	std::unordered_map<GLenum, std::string> shaderSources;
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);//find first "#type"
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);//find end of that line
			HREALENGINE_CORE_DEBUGBREAK(eol != std::string::npos, "Syntax error in shader file");

			size_t begin = pos + typeTokenLength + 1;//start of the word after "#type " (assumes ONE space!)
			std::string type = source.substr(begin, eol - begin);//extract stage name ("vertex"/"fragment")
			HREALENGINE_CORE_DEBUGBREAK(ShaderTypeFromString(type) != 0, "Unknown shader type '{0}'", type);

			
			size_t nextLinePos = source.find_first_not_of("\r\n", eol);//first char of the shader block (skip blank line break)
			pos = source.find(typeToken, nextLinePos);//find next "#type" (or npos if none)
			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - nextLinePos);//slice out the shader code between this line and the next "#type"
		}
		return shaderSources;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum,std::string>& shaderSources)
    {
		GLuint program = glCreateProgram();
		HREALENGINE_CORE_DEBUGBREAK(shaderSources.size() <= 2, "only 2 shaders are supported (vertex and fragment)");
		std::array<GLenum,2> glShaderIDs;
		int glShaderIDIndex = 0;
		for (auto& shaderSource : shaderSources)
		{
			GLenum shaderType = shaderSource.first;
			const std::string& source = shaderSource.second;

			GLuint shader = glCreateShader(shaderType);
			
			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, nullptr);
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if(isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
			
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
				LOG_CORE_ERROR("Shader Compile Error: {0}", infoLog.data());
			
				glDeleteShader(shader);
				return;
			}
			glAttachShader(program, shader);
			glShaderIDs[glShaderIDIndex++] = shader;
		}
		rendererID = program;
        glLinkProgram(program);
    	
        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
			LOG_CORE_ERROR("Program Linking error : {0}", infoLog.data());
			
            glDeleteProgram(program);
        	for (auto shaderID : glShaderIDs)
        		glDeleteShader(shaderID);
            return;
        }
		for (auto shaderID : glShaderIDs)
			glDetachShader(program, shaderID);
    }


    OpenGLShader::~OpenGLShader()
    {
    	glDeleteProgram(rendererID);
    }

    void OpenGLShader::Bind() const
    {
    	glUseProgram(rendererID);
    }

    void OpenGLShader::Unbind() const
    {
    	glUseProgram(0);
    }

    void OpenGLShader::SetInt(const std::string& name, int value)
    {
		UploadUniformInt(name, value);
    }

    void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
    {
		UploadUniformIntArray(name, values, count);
    }

    void OpenGLShader::SetFloat(const std::string& name, float value)
    {
		UploadUniformFloat(name, value);
    }

    void OpenGLShader::SetFloat3(const std::string& name,const glm::vec3& value)
    {
		UploadUniformFloat3(name, value);
    }

    void OpenGLShader::SetFloat4(const std::string& name,const glm::vec4& value)
    {
		UploadUniformFloat4(name, value);
    }

    void OpenGLShader::SetMat4(const std::string& name,const glm::mat4& value)
    {
		UploadUniformMat4(name, value);
    }

    void OpenGLShader::UploadUniformFloat(const std::string& uniformName, float values)
    {
    	GLint location = glGetUniformLocation(rendererID, uniformName.c_str());
    	glUniform1f(location, values);
    }

    void OpenGLShader::UploadUniformFloat2(const std::string& uniformName, const glm::vec2& values)
    {
    	GLint location = glGetUniformLocation(rendererID, uniformName.c_str());
    	glUniform2f(location, values.x, values.y);
    }

    void OpenGLShader::UploadUniformFloat3(const std::string& uniformName, const glm::vec3& values)
    {
    	GLint location = glGetUniformLocation(rendererID, uniformName.c_str());
    	glUniform3f(location, values.x, values.y, values.z);
    }

	void OpenGLShader::UploadUniformFloat4(const std::string& uniformName, const glm::vec4& values)
    {
    	GLint location = glGetUniformLocation(rendererID, uniformName.c_str());
    	glUniform4f(location, values.x, values.y, values.z, values.w);
    }

    void OpenGLShader::UploadUniformInt(const std::string& uniformName, int value)
    {
    	GLint location = glGetUniformLocation(rendererID, uniformName.c_str());
    	glUniform1i(location, value);
    }

    void OpenGLShader::UploadUniformIntArray(const std::string& uniformName, int* values, uint32_t count)
    {
		GLint location = glGetUniformLocation(rendererID, uniformName.c_str());
		glUniform1iv(location, count, values);
    }

    void OpenGLShader::UploadUniformMat3(const std::string& uniformName, const glm::mat3& matrix)
    {
    	GLint location = glGetUniformLocation(rendererID, uniformName.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }
	
    void OpenGLShader::UploadUniformMat4(const std::string& uniformName, const glm::mat4& matrix)
    {
    	GLint location = glGetUniformLocation(rendererID, uniformName.c_str());
    	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

  
}
