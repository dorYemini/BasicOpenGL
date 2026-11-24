#shader vertex
#version 330

layout(location = 0) in vec2 position;

out vec2 v_TexCoord;

void main()
{
	gl_Position = vec4(position, 0.0, 1.0);
	// Quad spans [-1,1]; map it to [0,1] so the whole image covers it
	v_TexCoord = position * 0.5 + 0.5;
}

#shader fragment
#version 330

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main()
{
	color = texture(u_Texture, v_TexCoord);
}
