
#version 330 core

in vec3 in_Position;
in vec2 in_UV;
out vec2 ex_UV;

void main(void) {
	gl_Position = vec4(in_Position.xy, 0.0, 1.0);
	ex_UV = vec2(in_UV.x, in_UV.y);
}

