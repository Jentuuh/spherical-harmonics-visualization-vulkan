#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(push_constant) uniform Push {
  mat4 modelMatrix; // projection * view * model
  mat4 normalMatrix;
  vec3 colorPush;
} push;

void main() {
  outColor = texture(texSampler, fragTexCoord);
	outColor = vec4(fragColor, 1.0);
} 