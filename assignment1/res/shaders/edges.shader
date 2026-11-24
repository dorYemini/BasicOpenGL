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

// float luma(vec2 uv)
// {
// 	// Clamp so border pixels don't wrap around (texture uses GL_REPEAT)
// 	vec2 halfTexel = 0.5 / vec2(textureSize(u_Texture, 0));
// 	uv = clamp(uv, halfTexel, 1.0 - halfTexel);
// 	return dot(texture(u_Texture, uv).rgb, vec3(0.299, 0.587, 0.114));
// }

// void main()
// {
// 	vec2 t = 1.0 / vec2(textureSize(u_Texture, 0));

// 	float tl = luma(v_TexCoord + t * vec2(-1,  1));
// 	float  l = luma(v_TexCoord + t * vec2(-1,  0));
// 	float bl = luma(v_TexCoord + t * vec2(-1, -1));
// 	float  u = luma(v_TexCoord + t * vec2( 0,  1));
// 	float  d = luma(v_TexCoord + t * vec2( 0, -1));
// 	float tr = luma(v_TexCoord + t * vec2( 1,  1));
// 	float  r = luma(v_TexCoord + t * vec2( 1,  0));
// 	float br = luma(v_TexCoord + t * vec2( 1, -1));

// 	// Sobel operator
// 	float gx = (tr + 2.0 * r + br) - (tl + 2.0 * l + bl);
// 	float gy = (tl + 2.0 * u + tr) - (bl + 2.0 * d + br);
// 	float edge = clamp(length(vec2(gx, gy)), 0.0, 1.0);

// 	color = vec4(vec3(edge), 1.0);
// }

void main()
{
	float value = 0.0;
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			int weight = (1 + (x & 1)) * (1 + (y & 1));
			vec2 coord = v_TexCoord + vec2((x - 1) * 1.0/256, (y - 1) * 1.0/256);
			value += weight * dot(texture(u_Texture, coord).rgb, vec3(0.299, 0.587, 0.114));
		}
	}

	vec3 c = texture(u_Texture, v_TexCoord).rgb;
	float gray = dot(c, vec3(0.299, 0.587, 0.114));

	color = vec4(vec3(value/16 - gray), 1.0);
}
