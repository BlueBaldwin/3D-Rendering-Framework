#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <string>

#include "Utilities.h"
#include "ShaderUtil.h"

// Single instance of ShaderUtil class - can be accessed anywhere without needing a pointer to the class object

// static Instance of ShaderUtil
ShaderUtil* ShaderUtil::mInstance = nullptr;
//Singleton Creation, Fetch and Destroy functionality
ShaderUtil* ShaderUtil::GetInstance()
{
	if (mInstance == nullptr)
	{
		return ShaderUtil::CreateInstance();
	}
	return mInstance;
}

ShaderUtil* ShaderUtil::CreateInstance()
{
	if (mInstance == nullptr)
	{
		mInstance = new ShaderUtil();
	}
	else
	{
		//Print to console that attempt to create multiple instance of ShaderUtil
		std::cout << "Attempt to create multiple Instances of ShaderUtil" << std::endl;
	}
	return mInstance;
}

void ShaderUtil::DestroyInstance()
{
	if (mInstance != nullptr)
	{
		delete mInstance;
		mInstance = nullptr;
	}
	else
	{
		// Print to console that attempt to destyroy null instance of ShaderUtil
		std::cout << "Attempt to destroy null instance of Shader Util" << std::endl;
	}
}
// Private constructor
ShaderUtil::ShaderUtil()
{
}

//Private destructor 
ShaderUtil::~ShaderUtil()
{
	// Delete any shaders that have not been unloaded
	for (auto iter = mShaders.begin(); iter != mShaders.end(); ++iter)
	{
		glDeleteShader(*iter);
	}
	// Destroy any programs that are still dangling about
	for (auto iter = mPrograms.begin(); iter != mPrograms.end(); ++iter)
	{
		glDeleteProgram(*iter);
	}
}

// Loading shaders from a file
unsigned int ShaderUtil::loadShader(const char* a_filename, unsigned int a_type)
{
	ShaderUtil* instance = ShaderUtil::GetInstance();
	return instance->loadShaderInternal(a_filename, a_type);
}

unsigned int ShaderUtil::loadShaderInternal(const char* a_filename, unsigned int a_type)
{
	// Integer to test for shader creation success
	int success = GL_FALSE;
	// Grab the shader source from the file
	char* source = Utilities::fileToBuffer(a_filename);
	unsigned int shader = glCreateShader(a_type);
	// Set the source buffer for the shader
	glShaderSource(shader, 1, (const char**)&source, 0);
	glCompileShader(shader);
	// as the buffer from fileToBuffer was allocated this needs to be destroyed
	delete[] source;

	// Test shader compilation for any errors and display them to console
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (GL_FALSE == success) // shader compilation failed, get logs and display them to console
	{
		int infoLogLength = 0;	// Variable to store the length of the error log
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength]; // allocate buffer to hold data
		glGetShaderInfoLog(shader, infoLogLength, 0, infoLog);
		std::cout << "Unable to compile: " << a_filename << std::endl;
		std::cout << infoLog << std::endl;
		delete[] infoLog;
		return 0;
	}
	// Success - add shader to mShaders vector
	mShaders.push_back(shader);
	return shader;
}
// Deleteiong the shaders 
void ShaderUtil::deleteShader(unsigned int a_shader)
{
	ShaderUtil* instance = ShaderUtil::GetInstance();
	instance->deleteShaderInternal(a_shader);
}

void ShaderUtil::deleteShaderInternal(unsigned int a_shader)
{
	//Loop through the shaders vector
	for (auto iter = mShaders.begin(); iter!= mShaders.end(); ++iter)
	{
		if (*iter == a_shader) // if we find the shader we are looking for
		{
			glDeleteShader(*iter);		// Delete the shader
			mShaders.erase(iter);		// Remove this item from the shaders vector
			break;
		}
	}
}

unsigned int ShaderUtil::createProgram(const int& a_vertexShader, const int& a_fragmentShader)
{
	ShaderUtil* instance = ShaderUtil::GetInstance();
	return instance->createProgramInternal(a_vertexShader, a_fragmentShader);
}

unsigned int ShaderUtil::createProgramInternal(const int& a_vertexShader, const int& a_fragmentShader)
{
	//boolean value to test for shader program linkage
	int sucess = GL_FALSE;

	// Create a shader program and attach the shaders to it
	unsigned int handle = glCreateProgram();
	glAttachShader(handle, a_vertexShader);
	glAttachShader(handle, a_fragmentShader);
	// link the shaders together into one shader program
	glLinkProgram(handle);
	// test to see if the program was successfully created
	glGetProgramiv(handle, GL_LINK_STATUS, &sucess);
	if (GL_FALSE == sucess) // if something has gone wrong then execute the following
	{
		int infoLogLength = 0; //Integer value to tell us the length of the error log
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
		// alocate enough space in the buffer for the error message
		char* infoLog = new char[infoLogLength];
		//fill the buffer with data 
		glGetProgramInfoLog(handle, infoLogLength, 0, infoLog);
		// print log message to console
		std::cout << "Shader Linker Error" << std::endl;
		std::cout << infoLog << std::endl;

		// delete the char buffer now we have displayed it
		delete[] infoLog;
		return 0; // return 0, programID 0 is a null program
	}
	// add the program to the shader program vector
	mPrograms.push_back(handle);
	return handle; // return the progam ID
}

void ShaderUtil::deleteProgram(unsigned int a_program)
{
	ShaderUtil* instance = ShaderUtil::GetInstance();
	instance->deleteProgramInternal(a_program);
}
void ShaderUtil::deleteProgramInternal(unsigned int a_program)
{
	// Loop through the shaders vector
	for (auto iter = mPrograms.begin(); iter != mPrograms.end(); ++iter)
	{
		if (*iter == a_program)			// if we find the shader we are looking for
		{
			glDeleteProgram(*iter);		// delete the shader
			mPrograms.erase(iter);		// remove this item from the shaders vector
			break; 
		}
	}
}

