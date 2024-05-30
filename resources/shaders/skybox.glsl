#stage vertex
#version 460 core

layout(location = 0) in vec3 position;

uniform mat4 u_viewProj;
uniform mat4 u_model;

out vec3 _uv;


void main()
{
    _uv  = position;
    vec4 outPos = u_viewProj * u_model * vec4(position, 1.0);
    gl_Position = outPos.xyww;
}  

#stage fragment
#version 460 core

in vec3 _uv;

out vec4 fragColor;

uniform samplerCube u_skymap;

void main()
{    
    vec3 color =texture(u_skymap, _uv).rgb;
    color = color / (color + vec3(1.0));

    const float GAMMA = 2.2;
    color = pow(color, vec3(1.0 / GAMMA));

    fragColor = vec4(color,1.0);
}
