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

const float PI = 3.14159265359;

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;

const float weak_threshold = 0.5;
const float strong_threshold = 0.75;

void main()
{
	float step = 1.0 / textureSize(u_Texture, 0).r;
	vec2 der = texture(u_Texture, v_TexCoord).xy;
	float mag = length(der.xy);
	float theta = atan(der.y / der.x);

	vec2 dir;
	if (theta >= -PI/8 && theta < PI/8) {
		dir = vec2(1.0, 0.0);
	} else if (theta >= PI/8 && theta < 3*PI/8) {
		dir = vec2(1.0, 1.0);
	} else if (theta >= -3*PI/8 && theta < -PI/8) {
		dir = vec2(-1.0, 1.0);
	} else {
		dir = vec2(0.0, 1.0);
	}

	if (mag > length(texture(u_Texture, v_TexCoord + dir*step).xy) &&
		mag > length(texture(u_Texture, v_TexCoord - dir*step).xy) &&
		mag > weak_threshold) {
		if (mag > strong_threshold) {
			color = vec4(1.0, 1.0, 1.0, 1.0);
		} else {
			color = vec4(0.5, 0.5, 0.5, 1.0);
		}
	} else {
		color = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
