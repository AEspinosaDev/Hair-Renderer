#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 2) in vec3 tangent;


uniform mat4 u_model;

out vec3 v_tangent;


void main() {
    gl_Position =  u_model * vec4(position, 1.0);
    v_tangent = normalize(mat3(transpose(inverse(u_model))) * tangent);
}

#shader geometry
#version 460 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 v_tangent[];

layout (binding = 1) uniform Scene
{
    vec4 ambient;
    vec4 lightPos;
    vec4 lightColor;
    vec4 shadowConfig;
    mat4 lightViewProj;
}u_scene;


out vec3 g_pos;

uniform float u_thickness;
uniform vec3 u_camPos;
uniform mat4 u_model;

void emitQuadPoint(vec4 origin,vec4 right, float offset){
    gl_Position =  u_scene.lightViewProj *(origin + right * offset);
    g_pos = gl_Position.xyz;
    EmitVertex();
}

void main() {
  
        vec4 startPoint = gl_in[0].gl_Position;
        vec4 endPoint = gl_in[1].gl_Position;

        vec4 view0 = vec4(u_camPos,1.0)-startPoint;
        vec4 view1 = vec4(u_camPos,1.0)-endPoint;

        vec3 dir0 = v_tangent[0];
        vec3 dir1 = v_tangent[1];

        vec4 right0 = normalize(vec4(cross(dir0.xyz,view0.xyz),0.0));
        vec4 right1 = normalize(vec4(cross(dir1.xyz,view1.xyz),0.0));

        vec3 normal0 = normalize(cross(right0.xyz,dir0.xyz));
        vec3 normal1 = normalize(cross(right1.xyz,dir1.xyz));

        float halfLength = u_thickness*0.5;

        emitQuadPoint(startPoint,right0,halfLength);
        emitQuadPoint(endPoint,right1,halfLength);
        emitQuadPoint(startPoint,-right0,halfLength);
        emitQuadPoint(endPoint,-right1,halfLength);

}

#shader fragment
#version 460 core

in vec3 g_pos;

void main() {
    // gl_FragDepth = 0.0f;
    //void..
}