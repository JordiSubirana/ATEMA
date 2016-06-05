#version 330 core

in vec2 tex_coord0;
uniform sampler2D custom_texture;

void main()
{
	vec4 c = texture2D(custom_texture, tex_coord0.xy);
	gl_FragColor = c;
}