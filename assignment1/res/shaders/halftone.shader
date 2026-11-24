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

const float halftone_patterns[20] = float[20](
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 1.0, 1.0, 0.0,
	0.0, 1.0, 1.0, 1.0,
	1.0, 1.0, 1.0, 1.0
);

void main()
{
    ivec2 size = textureSize(u_Texture, 0);
	vec3 c = texelFetch(u_Texture, ivec2(v_TexCoord * size), 0).rgb;
	float gray = dot(c, vec3(0.299, 0.587, 0.114));
	// Converts [0, 1) to [0, 256), output is 512 so fractional part is either {0, 0.5} per axis
	ivec2 offset = ivec2(fract(v_TexCoord * size) * 2);
	int col = int(offset.y * 2 + offset.x);
	int row = min(int(gray / 0.2), 4);
	color = vec4(vec3(halftone_patterns[row * 4 + col]), 1.0);
}
