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

uniform sampler2D u_Texture; // previous pass's output (blurred grayscale)

void main()
{
	float step = 1.0 / textureSize(u_Texture, 0).r;

	float dx = 0.0;
	dx += texture(u_Texture, v_TexCoord + vec2(-1, -1)*step).r;
	dx += 2*texture(u_Texture, v_TexCoord + vec2(-1, 0)*step).r;
	dx += texture(u_Texture, v_TexCoord + vec2(-1, 1)*step).r;
	dx -= texture(u_Texture, v_TexCoord + vec2(1, -1)*step).r;
	dx -= 2*texture(u_Texture, v_TexCoord + vec2(1, 0)*step).r;
	dx -= texture(u_Texture, v_TexCoord + vec2(1, 1)*step).r;
	float dy = 0.0;
	dy += texture(u_Texture, v_TexCoord + vec2(-1, -1)*step).r;
	dy += 2*texture(u_Texture, v_TexCoord + vec2(0, -1)*step).r;
	dy += texture(u_Texture, v_TexCoord + vec2(1, -1)*step).r;
	dy -= texture(u_Texture, v_TexCoord + vec2(-1, 1)*step).r;
	dy -= 2*texture(u_Texture, v_TexCoord + vec2(0, 1)*step).r;
	dy -= texture(u_Texture, v_TexCoord + vec2(1, 1)*step).r;

	color = vec4(dx, dy, 0.0, 1.0);
}
