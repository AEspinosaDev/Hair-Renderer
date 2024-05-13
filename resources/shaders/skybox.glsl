#stage vertex
#version 460 core

layout(location = 0) in vec3 position;

uniform mat4 u_viewProj;

out vec3 _uv;


void main()
{
    _uv  = position;
    vec4 outPos = u_viewProj * vec4(position, 1.0);
    gl_Position = outPos.xyww;
}  

#stage fragment
#version 460 core

in vec3 _uv;

out vec4 fragColor;

uniform samplerCube u_skymap;

void main()
{    
     fragColor = vec4(texture(u_skymap, _uv).rgb,1.0);
}
