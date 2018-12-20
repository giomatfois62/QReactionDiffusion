#version 330 core
in vec2 texCoord;
in vec3 normal;
in vec3 color;

out vec4 Color;

uniform sampler2D screenTexture;
uniform bool useTexture;

void main(void)
{
    vec3 light = vec3(0.0f, 0.0f, -1.0f);
    float diffuse = max(dot(light, normal), 0);
    //Color = vec4(vec3(0.5f, 0.3f, 0.8f) , 1.0f);
    //Color = vec4(diffuse*color, 1.0f);
    //Color = int(useTexture) * texture(screenTexture, texCoord);
    Color = int(useTexture) * diffuse * texture(screenTexture, texCoord) + int(!useTexture) * vec4(diffuse*color, 1.0f);
    Color.w = 1.0f;
}
