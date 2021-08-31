#version 450

layout(set = 0, binding = 0) uniform FrameData
{
    mat4 proj;
    mat4 view;
} frameData;

layout(set = 1, binding = 0) uniform ObjectData
{
	mat4 model;
} objectData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragPosition;

void main()
{
	vec4 position = frameData.proj * frameData.view * objectData.model * vec4(inPosition, 1.0);
	
    gl_Position = position;
	
    fragColor = inColor;
    fragTexCoord = inTexCoord;
	fragPosition = position.xyz;
}

