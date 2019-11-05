#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;

uniform sampler2D myTextureSampler;

void main(){
	color = texture( myTextureSampler, UV ).rgb;
}