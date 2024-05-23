#stage vertex
#version 460 core

layout(location = 0) in vec3 position;

layout (binding = 1) uniform Scene
{
    vec4 ambient;
    vec4 lightPos;
    vec4 lightColor;
    vec4 shadowConfig;
    mat4 lightViewProj;
}u_scene;

uniform mat4 u_model;


void main() {
    gl_Position = u_scene.lightViewProj  * u_model * vec4(position, 1.0);
}

#stage fragment
#version 460 core

uniform bool u_isHair;

out vec2 fragColor;

void main() {

    fragColor = u_isHair ? vec2(0.0,gl_FragCoord.z) : vec2(gl_FragCoord.z,0.0);
}