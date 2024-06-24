#stage vertex
#version 460

layout(location = 0) in vec3 position;
layout(location = 3) in vec2 uv;


out vec2 v_uv;
out vec4 v_pos;

uniform vec2 u_screen;

// This has to be a big triangle



void main() {
    gl_Position = vec4(position, 1.0);
    v_uv = uv;
    v_pos = vec4(position, 1.0);
}

#stage fragment
#version 460

#define SMAAx2

in vec2 v_uv;
in vec4 v_pos;

uniform sampler2DMS u_frame;

layout(location = 0) out vec4 outColor0;
layout(location = 1) out vec4 outColor1;



void main() {
    
    ivec2 texelCoords =  ivec2(v_uv*textureSize(u_frame));
    outColor0 = texelFetch(u_frame, texelCoords, 0);
    outColor1 = texelFetch(u_frame, texelCoords, 1);
}
