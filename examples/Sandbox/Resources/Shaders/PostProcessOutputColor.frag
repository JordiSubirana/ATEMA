#version 450

layout(set = 0, binding = 0) uniform sampler2D gbufferPosition;
layout(set = 0, binding = 1) uniform sampler2D gbufferNormal;
layout(set = 0, binding = 2) uniform sampler2D gbufferColor;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = texture(gbufferColor, fragTexCoord);
}
