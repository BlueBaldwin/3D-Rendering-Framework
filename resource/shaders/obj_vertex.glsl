//\------------------------------------------------------------------------------------------
//\ Out putting the vertex normal data for the fragment shader
//\------------------------------------------------------------------------------------------
#version 400 //We want to use open GL Syntax

//Declaring the input data
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 uvCoord;

smooth out vec4 vertPos;
smooth out vec4 vertNormal;
smooth out vec2 vertUV;

// Send the projectionViewMatrix to the vertex shader via the uniform variable
uniform mat4 ProjectionViewMatrix;
uniform mat4 ModelMatrix;

// Main function will set the Vertex position to whatever was in the buffer
void main()
{
    // Just set the colour to grey
	vertUV = uvCoord;
	vertNormal = normal;
	vertPos = ModelMatrix * position;   // World space position
	gl_Position = ProjectionViewMatrix * ModelMatrix * position; // Screenspace position
}
