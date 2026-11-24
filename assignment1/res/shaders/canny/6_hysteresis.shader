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
	float step = 1.0 / textureSize(u_Texture, 0).x;

	if (texture(u_Texture, v_TexCoord).r == 0.5) {
		for (int x = -1; x < 2; x++) {
			for (int y = -1; y < 2; y++) {
				if (texture(u_Texture, v_TexCoord + vec2(x, y)).r == 1.0) {
					color = vec4(1.0, 1.0, 1.0, 1.0);
 					return;
				}
			}
		}
		color = vec4(0.0, 0.0, 0.0, 1.0);
	} else {
		color = texture(u_Texture, v_TexCoord);
	}
}
