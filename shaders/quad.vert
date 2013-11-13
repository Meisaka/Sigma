#version 330 core

in vec3 in_Position;
in vec2 in_UV;
uniform int enable_projection;
uniform mat4 in_View;
uniform mat4 in_Proj;
out vec2 ex_UV;

void main(void){
	if(enable_projection == 0) {
	    gl_Position = vec4(in_Position.xy, 0, 1);
	}
	else {
	    gl_Position = in_Proj * (in_View * vec4(in_Position.xy, 0, 1));
	}
	ex_UV = in_UV;
}