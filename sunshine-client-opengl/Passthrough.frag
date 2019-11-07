#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;

uniform sampler2D ySampler;
uniform sampler2D uSampler;
uniform sampler2D vSampler;

const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);
const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);
const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);
const vec3 offset = vec3(-0.0625, -0.5, -0.5);

void main(){
	float y = texture(ySampler, UV).r;
	float u = texture(uSampler, UV).r;
	float v = texture(vSampler, UV).r;

	vec3 yuv = vec3(y, u, v);
	yuv += offset;

	color.r = dot(yuv, R_cf);
	color.g = dot(yuv, G_cf);
	color.b = dot(yuv, B_cf);
}
