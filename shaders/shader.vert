#version 450
//#extension GL_KHR_vulkan_glsl : enable


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPos;

layout(set = 0, binding = 0) uniform UniformBufferObject{
    vec2 foo;
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    vec4 worldPosition = ubo.model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * worldPosition;

    fragTexCoord = inTexCoord;
    fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal; // Transform normal to world space
    fragPos = worldPosition.xyz; // Pass world-space position to fragment shader
}
