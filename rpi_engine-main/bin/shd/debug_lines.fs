#version 310 es
 
precision highp float;
precision highp int;

in vec4 fragColor;

out vec4 diffuseColor;

void main()
{
	diffuseColor = fragColor;
}
