#stage vertex
#version 460

layout(location = 0) in vec3 position;
layout(location = 3) in vec2 uv;


out vec2 v_uv;

void main() {
    gl_Position = vec4(position, 1.0);
    v_uv = uv;
}

#stage fragment
#version 460

in vec2 v_uv;

uniform sampler2D u_frame;

out vec4 aaOutput;
