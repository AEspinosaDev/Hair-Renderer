#shader vertex
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

#shader fragment
#version 460 core

void main() {
    //void
}