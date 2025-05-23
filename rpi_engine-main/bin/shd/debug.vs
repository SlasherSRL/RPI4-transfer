#version 310 es
precision highp float;
precision highp int;

layout(location=0) in vec3 position;

uniform mat4 model;
uniform mat4 viewProjection;

void main()
{
	gl_Position = viewProjection * model * vec4(position, 1.0f);
}
