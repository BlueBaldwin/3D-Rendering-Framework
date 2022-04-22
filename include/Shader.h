#pragma once

class Shader
{

public:
	static GLuint CreateShader(const char*, unsigned int);
	GLuint static CreateProgram();

private:
	// Private Constructor and Destructor
	Shader();
	~Shader();
};