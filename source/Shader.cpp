#include <glad/glad.h>
#include <fstream>
#include <string>
#include <vector>

#include "Shader.h"

GLuint Shader::CreateShader(const char* a_strShaderFile, unsigned int a_eShaderType)
{
	std::string strShaderCode;
	//open shader file
	std::ifstream shaderStream(a_strShaderFile);

	//if that worked ok, load file line by line
	if (shaderStream.is_open())
	{
		std::string Line = "";
		while (std::getline(shaderStream, Line))
		{
			strShaderCode += "\n" + Line;
		}
		shaderStream.close();
	}

	//convert to cstring
	char const* szShaderSourcePointer = strShaderCode.c_str();

	//create shader ID
	GLuint uiShader = glCreateShader(a_eShaderType);
	//load source code
	glShaderSource(uiShader, 1, &szShaderSourcePointer, NULL);

	//compile shader 
	glCompileShader(uiShader);

	//check for compilation errors and output them
	GLint iStatus;
	glGetShaderiv(uiShader, GL_COMPILE_STATUS, &iStatus);
	if (iStatus == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(uiShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength];
		glGetShaderInfoLog(uiShader, infoLogLength, NULL, strInfoLog);

		const char* strShaderType = NULL;
		switch (a_eShaderType)
		{
		case GL_VERTEX_SHADER: strShaderType = "vertex";
			break;

		case GL_FRAGMENT_SHADER: strShaderType = "fragment";
			break;
		}
		fprintf(stderr, "Compile failure in %s shader:\n%s\n", strShaderType,
			strInfoLog);
		delete[] strInfoLog;
	}
	return uiShader;
}

GLuint Shader::CreateProgram()
{
	std::vector<GLuint> shaderList;
	shaderList.push_back(CreateShader("resource/shaders/vertex.glsl", GL_VERTEX_SHADER));
	shaderList.push_back(CreateShader("resource/shaders/fragment.glsl", GL_FRAGMENT_SHADER));

	//create shader program ID
	GLuint uiProgram = glCreateProgram();

	//attach shaders
	for (auto shader = shaderList.begin(); shader != shaderList.end(); shader++)
		glAttachShader(uiProgram, *shader);

	//link program
	glLinkProgram(uiProgram);

	//check for link errors and output them
	GLint status;
	glGetProgramiv(uiProgram, GL_LINK_STATUS, &status); if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(uiProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* strInfoLog = new GLchar[infoLogLength]; glGetProgramInfoLog(uiProgram, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}
	for (auto shader = shaderList.begin(); shader != shaderList.end(); shader++)
	{
		glDetachShader(uiProgram, *shader);
		glDeleteShader(*shader);
	}
	return uiProgram;
}

Shader::~Shader()
{
}