#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

void main()
{
	vec4 position = vec4(inPosition, 1.0);
	
    gl_Position = position;
	
    fragTexCoord = inTexCoord;
}

