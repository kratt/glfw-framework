#version 400 core

in vec4 VertPosition;
in vec4 VertNormal;
in vec4 VertColor;
in vec4 VertTexture;

out vec4 FragData;

uniform vec3 lightPos;

void main()
{
   FragData = vec4(0, 0, 0, 1);	
}