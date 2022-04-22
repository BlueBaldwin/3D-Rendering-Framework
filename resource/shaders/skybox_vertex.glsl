#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 ProjectionViewMatrix;
uniform mat4 view;

void main()
{
	TexCoords = vec3(vec4(aPos, 1.0));
	vec4 pos = ProjectionViewMatrix * view * vec4(aPos, 1.0);
	gl_Position = pos.xyww;
}