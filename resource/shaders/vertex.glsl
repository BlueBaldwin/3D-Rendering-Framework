#version 400 //We want to use open GL Syntax

//Declaring the input data
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 colour;

smooth out vec4 vertColour;

// Send the projectionViewMatrix to the vertex shader via the uniform variable
uniform mat4 ProjectionViewMatrix;

// Main function will set the Vertex position to whatever was in the buffer
void main()
{
	vertColour = colour;
	gl_Position = ProjectionViewMatrix * position; // gl_Position is a built in variable to Open GL, YOU must asign it with a value
}

