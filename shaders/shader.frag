#version 450
//#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_spirv_intrinsics : enable

layout(location = 0)  in vec2 fragTexCoord;
layout(location = 1)  in vec3 fragNormal;
layout(location = 2)  in vec3 fragPos;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(set = 2, binding = 0) uniform UniformBufferObject{
    	vec3 ks;
		vec3 cameraPos;
		vec3 lightPos;
		float lightIntensity;
} ubo;

void main()
{

    vec3 color = texture(texSampler, fragTexCoord).rgb;

//    debugPrintfEXT("cameraPos is %v3f", ubo.cameraPos);
//    debugPrintfEXT("lightPos is %v3f", ubo.lightPos);
//    debugPrintfEXT("lightIntensity is %f", ubo.lightIntensity);

    vec3 ambient = 0.05 * color;

    vec3 lightDir = normalize(ubo.lightPos - fragPos);
    vec3 normal = normalize(fragNormal);

    float diff = max(dot(lightDir, normal), 0.0);
    float light_atten_coff = ubo.lightIntensity / max(pow(length(ubo.lightPos - fragPos), 2.0), 1.0);
       // debugPrintfEXT("light_atten_coff is %v3f", ubo.ks);
    vec3 diffuse = diff * color * light_atten_coff;

    vec3 viewDir = normalize(ubo.cameraPos - fragPos);
    float spec = 0.0;

        vec3 reflectDir = reflect(-lightDir, normal); // Negate lightDir for correct reflection
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 35.0);

    vec3 specular = ubo.ks * spec * light_atten_coff;

    outColor = vec4((specular + diffuse + ambient), 1.0);

}
