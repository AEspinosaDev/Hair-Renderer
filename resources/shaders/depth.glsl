#shader vertex
#version 460 core

layout(location = 0) in vec3 position;

layout (binding = 0) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;
}u_camera;

uniform mat4 u_model;

out vec3 _pos;

void main() {
    
    _pos = (u_camera.modelView * vec4(position, 1.0)).xyz;

    gl_Position = u_camera.viewProj  * u_model * vec4(position, 1.0);

}

#shader fragment
#version 460 core

void main() {
    //void
}