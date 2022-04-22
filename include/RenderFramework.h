#pragma once

#include <glm/glm.hpp>

#include "Application.h"
#include <ApplicationEvent.h>
//Forward declare OBJ model

class OBJModel;

class RenderFramework : public Application
{
public:
	RenderFramework();
	virtual ~RenderFramework();

	void onWindowResize(WindowResizeEvent* e);
	const glm::mat4& GetCameraMatrix() { return m_cameraMatrix; };
	void ChangeBackgroundColour(glm::vec3* a_backgroundColour, glm::vec3* a_specularTint);
	void SaveBackgroundColour(glm::vec3 &a_backgroundColour, glm::vec3 a_newBackgroundColour);
	void MainMenu(bool& m_bMy_tool_active);

protected:
	virtual bool onCreate();
	virtual void Update(float deltaTime);
	virtual void Draw();
	virtual void Destroy();

private:
	// Structure for a simple vertex - interleaved (position, colour)
	typedef struct Vertex
	{
		glm::vec4 position;
		glm::vec4 colour;
	}Vertex;

	// Structure for a line
	typedef struct Line
	{
		Vertex v0;
		Vertex v1;
	}Line;

	Line* m_lines;
	glm::mat4 m_cameraMatrix;
	glm::mat4 m_projectionMatrix;

	unsigned int m_uiProgram;
	unsigned int m_objProgram; // Variable for the shader program
	unsigned int m_lineVBO;
	unsigned int m_objModelBuffer[2]; // Used for the index and vertex buffer of the model

	// Model
	OBJModel* m_objModel;
	Line* lines;
	glm::vec3 m_specularTint;
	glm::vec3 m_backgroundColour;


	// Skybox rendering 
	unsigned int m_CubeMaptextID;
	unsigned int m_SBVAO;
	unsigned int m_SBVBO;
	unsigned int m_SBProgramID;

	// ImGui
	bool m_bMy_tool_active = true;
	bool m_changeColour = false;
};





