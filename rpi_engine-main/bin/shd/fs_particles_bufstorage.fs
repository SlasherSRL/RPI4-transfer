#version 310 es
 
precision highp float;
precision highp int;

in vec4 Color;
out vec4 out_Color;

void main()
{
	out_Color = Color;
}
