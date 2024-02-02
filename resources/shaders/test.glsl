#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 uv;
layout(location = 4) in vec3 color;

uniform mat4 u_viewProj;
uniform mat4 u_model;
uniform mat4 u_view;
uniform vec3 u_lightPos;

out vec3 v_lightPos;
out vec3 v_color;

void main() {
    gl_Position = u_viewProj * u_model * vec4(position, 1.0);

    v_color = color;
    v_lightPos = (u_view * vec4(u_lightPos,1.0)).xyz;
}

#shader geometry
#version 460 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 v_color[];
in vec3 v_lightPos[];

out vec3 g_color;
out vec3 g_lightPos;

out vec3 g_pos;
out vec3 g_normal;
out vec3 g_tangent;

uniform float u_thickness;

void main() {
        vec4 startPoint = gl_in[0].gl_Position;
        vec4 endPoint = gl_in[1].gl_Position;

        vec4 strandDirection = normalize(endPoint - startPoint);

        vec4 startViewDir = -startPoint;
        vec4 endViewDir = -endPoint;

        vec3 crossStartNormal =normalize(cross(strandDirection.xyz,startViewDir.xyz));
        vec3 crossEndNormal = normalize(cross(strandDirection.xyz,endViewDir.xyz));
        vec4 startNormal = vec4(crossStartNormal,0.0);
        vec4 endNormal = vec4(crossEndNormal,0.0);

        float halfLength = u_thickness*0.5f;

        gl_Position = startPoint + startNormal * halfLength;
        g_pos = gl_Position.xyz;
        g_color = v_color[0];
        g_lightPos = v_lightPos[0];
        g_normal = normalize(cross(startNormal.xyz,strandDirection.xyz));
        g_tangent = strandDirection.xyz;
        EmitVertex();

        gl_Position = endPoint + endNormal * halfLength;
        g_pos = gl_Position.xyz;
        g_color = v_color[1];
         g_lightPos = v_lightPos[0];
        g_normal = normalize(cross(endNormal.xyz,strandDirection.xyz));
        g_tangent = strandDirection.xyz;
        EmitVertex();

        gl_Position = startPoint - startNormal * halfLength;
        g_pos = gl_Position.xyz;
        g_color = v_color[0];
         g_lightPos = v_lightPos[0];
        g_normal = normalize(cross(startNormal.xyz,strandDirection.xyz));
        g_tangent = strandDirection.xyz;
        EmitVertex();

        gl_Position = endPoint - endNormal * halfLength;
        g_pos = gl_Position.xyz;
        g_color = v_color[1];
         g_lightPos = v_lightPos[0];
        g_normal = normalize(cross(endNormal.xyz,strandDirection.xyz));
        g_tangent = strandDirection.xyz;
        EmitVertex();
        EndPrimitive();


}

#shader fragment
#version 460 core

in vec3 g_color;
in flat vec3 g_lightPos;

in vec3 g_pos;
in vec3 g_normal;
in vec3 g_tangent;

out vec4 FragColor;

vec3 phong() {

    vec3 lightDir = normalize(g_lightPos- g_pos);
    vec3 viewDir = normalize(-g_pos);
    vec3 halfVector = normalize(lightDir + viewDir);

    vec3 diffuse = clamp(dot(lightDir, g_normal), 0.0, 1.0) * vec3(1.0,1.0,1.0);

    //Blinn specular term
    vec3 specular = pow(max(dot(g_normal, halfVector), 0.0), 20.0) * 5.0 * vec3(1.0,1.0,1.0);

    vec3 color =  vec3(1.0,0.0,0.0); //surface
    
    // float att = computeAttenuation();
     float att = 1.0;


    return (diffuse + specular) * g_color * att * 1.0;

}

void main() {
    FragColor = vec4(g_color, 1.0);
}