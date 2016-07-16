#version 330 core

//ATEMA types definition
struct AtemaMaterial
{
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
	
	float shininess;
	
	sampler2D ambient_texture;
	sampler2D diffuse_texture;
	sampler2D specular_texture;
};

struct AtemaModel
{
	vec3 position;
	vec3 normal;
	vec2 tex_coords;
};

struct AtemaPointLight
{
	vec3 position;
	vec4 color;
	float radius;
	float intensity;
};
//End of ATEMA types definition


in vec3 at_model_position;
in vec3 at_model_normal;
in vec2 at_model_tex_coords;
AtemaModel model = { at_model_position, at_model_normal, at_model_tex_coords };

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
