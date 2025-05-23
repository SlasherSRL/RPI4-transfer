#version 310 es
 
precision highp float;
precision highp int;

layout(location=0) in vec3 in_SampleDir;

layout(location=0) uniform samplerCube SkyboxTexture;

out vec4 out_Color;

void main()
{
	out_Color = vec4(texture(SkyboxTexture, in_SampleDir).rgb, 1.0f);
}
