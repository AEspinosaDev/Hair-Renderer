#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 3) in vec2 uv;

out vec2 _uv;

void main() {
    gl_Position = vec4(position, 1.0);
    _uv = uv;
}

#shader fragment
#version 460 core

in vec2 _uv;

uniform sampler2D u_frame;

out vec4 FragColor;

void main() {
    FragColor = vec4(texture(u_frame, _uv).rgb,0.5);
    //  FragColor = gl_Frag vec4(_uv.x,_uv.y,0.0,1.0);
}