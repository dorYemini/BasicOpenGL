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

uniform sampler2D u_Texture;

void main()
{
	vec3 c = texture(u_Texture, v_TexCoord).rgb;
	float value = dot(c, vec3(0.299, 0.587, 0.114));
	color = vec4(vec3(value), 1.0);
}
