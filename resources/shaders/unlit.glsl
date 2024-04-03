#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 3) in vec3 uv;
layout(location = 4) in vec3 color;

layout (std140) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;
}u_camera;

uniform mat4 u_model;

out vec3 _pos;
out vec3 _color;

void main() {
    
    _pos = (u_camera.modelView * vec4(position, 1.0)).xyz;

    _color = color;

    gl_Position = u_camera.viewProj  * u_model * vec4(position, 1.0);

}

#shader fragment
#version 460 core

in vec3 _pos;
in vec3 _color;

uniform vec3 u_baseColor;
uniform float u_opacity;
uniform bool u_useVertexColor;

out vec4 FragColor;


void main() {
    FragColor = vec4(!u_useVertexColor? u_baseColor : _color, u_opacity);
    FragColor = vec4(1.0,1.0,1.0, 1.0);


}