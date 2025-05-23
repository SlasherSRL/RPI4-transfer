#version 310 es
precision highp float;
precision highp int;

uniform vec4 color;
out vec4 diffuseColor;

void main()
{
	diffuseColor = color;
}
