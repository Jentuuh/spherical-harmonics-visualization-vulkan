#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout (set=0, binding = 0) uniform UBO 
{
  mat4 projectionMatrix;
  vec3 directionToLight;
  mat4 view;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix; // projection * view * model
  mat4 normalMatrix;
  vec3 colorPush;
} push;

const float AMBIENT = 0.02;



void main() {
  gl_Position = ubo.projectionMatrix * ubo.view * push.modelMatrix * vec4(position, 1.0);
  vec3 normalWorldSpace =  normalize(mat3(push.normalMatrix) * normal);

  // If light intensity is negative(surface isn't facing light), the intensity should be 0
  float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.directionToLight), 0);

  fragColor = lightIntensity * push.colorPush;
  fragTexCoord = uv;
}