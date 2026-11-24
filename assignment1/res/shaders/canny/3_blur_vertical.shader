#shader vertex
#version 330

layout(location = 0) in vec2 position;

out vec2 v_TexCoord;

void main()
{
	gl_Position = vec4(position, 0.0, 1.0);
	v_TexCoord = position * 0.5 + 0.5;
}

#shader fragment
#version 330

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_Texture; // previous pass's output (pass 1: the source image)

void main()
{
	float step = 1.0 / textureSize(u_Texture, 0).r;
	float value = 0.0;
	value += 0.25*texture(u_Texture, v_TexCoord + vec2(0, -step)).r;
	value += 0.5*texture(u_Texture, v_TexCoord).r;
	value += 0.25*texture(u_Texture, v_TexCoord + vec2(0, step)).r;
	color = vec4(vec3(value), 1.0);
}
