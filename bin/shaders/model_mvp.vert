#version 330 core

in AtemaModel model;

uniform mat4 mat_M;
uniform mat4 mat_V;
uniform mat4 mat_P;

void main()
{
	mat4 mvp = mat_P * mat_V * mat_M;
	
	vec4 pos = mvp * vec4(model.position.x, model.position.y, model.position.z, 1.0);
	// gl_Position = vec4(position.x, position.y, position.z, 1.0);
	gl_Position = pos;
}