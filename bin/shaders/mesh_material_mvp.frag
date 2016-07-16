#version 330 core

ATEMA_MODEL_FRAGMENT_DATA model;

uniform AtemaPointLight light01;
uniform vec3 camera_pos;

in vec3 position;
in vec3 normal;
in vec2 uv;

out vec4 out_color;

void main()
{
	vec4 color = vec4(0,0,0,1);
	
	vec3 light01_position = light01.position;
	vec4 light01_color = light01.color;
	float light01_intensity = light01.intensity;
	float light01_radius = light01.radius;
	
	light01_position = vec3(20,0,-20);
	light01_color = vec4(0.6,0.6,0.6,1.0);
	light01_intensity = 0.2;
	light01_radius = 100.0;
	
	//Diffuse
	vec3 light = normalize(light01_position - position);
	
	float cos_theta = clamp( dot(normalize(normal), light), 0, 1 );
	
	float distance = length(light01_position - position);
	float attenuation = distance / light01_radius;
	
	color = ((texture(model.diffuse_texture, uv.xy) + light01_color * light01_intensity) * cos_theta * (1.0-attenuation)) / (1.0+light01_intensity);
	
	//Specular
	vec3 e = normalize(camera_pos-position);
	vec3 r = reflect(-light, normalize(normal));
	float cos_alpha = clamp( dot(e, r), 0,1 );
	
	color += ((texture(model.specular_texture, uv.xy) + light01_color * light01_intensity) * pow(cos_alpha,2) * (1.0-attenuation)) / (1.0+light01_intensity);
	
	//Ambient
	color += 0.3*texture(model.diffuse_texture, uv.xy);
	
	out_color = color;
}