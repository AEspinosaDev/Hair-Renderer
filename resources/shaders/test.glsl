#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 uv;
layout(location = 4) in vec3 color;

uniform mat4 u_viewProj;
uniform mat4 u_model;

out vec3 v_color;

void main() {
    gl_Position = u_viewProj  * vec4(position, 1.0);
    // gl_Position =  vec4(position, 1.0);

    v_color = color;
}

#shader fragment
#version 460 core

in vec3 v_color;

out vec4 FragColor;

void main() {
    FragColor = vec4(v_color, 1.0);
}