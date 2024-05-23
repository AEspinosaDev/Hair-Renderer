#stage vertex
#version 460 core

layout(location = 0) in vec3 position;

layout (binding = 0) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;
}u_camera;


uniform mat4 u_model;


void main() {
    gl_Position = u_camera.viewProj  * u_model * vec4(position, 1.0);
}

#stage fragment
#version 460 core

out vec4 fragColor;

void main() {
    fragColor = vec4(0.0);
}
