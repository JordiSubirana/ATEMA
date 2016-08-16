#version 330 core

ATEMA_MODEL_VERTEX_DATA model;

uniform mat4 mat_M;
uniform mat4 mat_V;
uniform mat4 mat_P;

out vec3 position;
out vec3 normal;
out vec2 uv;

void main()
{
	mat4 mvp = mat_P * mat_V * mat_M;
	
	vec4 pos = mvp * vec4(model.position.x, model.position.y, model.position.z, 1.0);
	
	gl_Position = pos;
	
	position = pos.xyz;
	normal = (mvp * vec4(model.normal.x, model.normal.y, model.normal.z, 0.0)).xyz;
	uv = model.tex_coords;
}