#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform PointLight
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 viewPos;
} light;

void main()
{
    vec3 viewDir = normalize(light.viewPos - fragPos);

    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    float distance = length(light.position - fragPos);

    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;

    vec3 final = ambient + diffuse + specular;

    outColor = vec4(final, 1.0) * texture(texSampler, fragTexCoord);
}