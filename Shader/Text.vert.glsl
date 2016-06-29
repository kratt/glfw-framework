//Author: S�ren Pirk
//Date: 22.01.2013

#version 400 core

#define VERT_POSITION	0
#define VERT_NORMAL     1
#define VERT_COLOR		2
#define VERT_TEXTURE    3

uniform mat4x4 matModel;
uniform mat4x4 matView;
uniform mat4x4 matProjection;

layout(location = VERT_POSITION) in vec4 Position;
layout(location = VERT_NORMAL)   in vec4 Normal;
layout(location = VERT_COLOR)    in vec4 Color;
layout(location = VERT_TEXTURE)  in vec4 Texture;

out vec4 VertPosition;
out vec4 VertNormal;
out vec4 VertColor;
out vec4 VertTexture;

void main()
{	   
    VertPosition = Position; 
    VertNormal   = Normal;
	VertColor    = Color;
	VertTexture  = Texture;
	
	mat4 tmpView = matView * matModel;
	tmpView[0].xyz = vec3(1,0,0);
	tmpView[1].xyz = vec3(0,1,0);
	tmpView[2].xyz = vec3(0,0,1);
	//tmpView[3].xyz = Position.xyz;
	
    gl_Position = matProjection * tmpView *vec4(Position.xyz, 1);
}
