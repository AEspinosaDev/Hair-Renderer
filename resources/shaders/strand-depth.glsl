#stage vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 uv;
layout(location = 4) in vec3 color;


uniform mat4 u_model;

out vec3 v_color;
out vec3 v_tangent;
out int v_id;

void main() {

    gl_Position =  u_model * vec4(position, 1.0);

    v_tangent = normalize(mat3(transpose(inverse(u_model))) * tangent);
    v_color = color;
    v_id = gl_VertexID;

}

#stage geometry
#version 460 core


layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 v_color[];
in vec3 v_tangent[];
in int v_id[];

layout (binding = 0) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;

}u_camera;

out vec3 g_pos;
out vec3 g_modelPos;
out vec3 g_normal;
out vec2 g_uv;
out vec3 g_dir;
out vec3 g_color;
out vec3 g_origin;
out int g_id;

uniform float u_thickness;
uniform vec3 u_camPos;

void emitQuadPoint(vec4 origin, 
                  vec4 right,
                  float offset,
                  vec3 forward, 
                  vec3 normal, 
                  vec2 uv,
                  int id){
  
        vec4 newPos = origin + right * offset; //Model space
        gl_Position =  u_camera.viewProj * newPos;
        g_dir = normalize(mat3(transpose(inverse(u_camera.view))) * v_tangent[id]);
        g_color = v_color[id];
        g_pos = (u_camera.view *  newPos).xyz;
        g_modelPos = newPos.xyz;
        g_uv = uv;
        g_normal =  normalize(mat3(transpose(inverse(u_camera.view))) * normal);
        g_origin = (u_camera.view * origin).xyz; 
        g_id = v_id[0];

        EmitVertex();
}

void main() {
  
        //Model space --->>>

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

        //<<<----

        float halfLength = u_thickness*0.5;

        emitQuadPoint(startPoint,right0,halfLength,dir0,normal0,vec2(1.0,0.0),0);
        emitQuadPoint(endPoint,right1,halfLength,dir1,normal1,vec2(1.0,1.0),1);
        emitQuadPoint(startPoint,-right0,halfLength,dir0,normal0,vec2(0.0,0.0),0);
        emitQuadPoint(endPoint,-right1,halfLength,dir1,normal1,vec2(0.0,1.0),1);

}

#stage fragment
#version 460 core


in vec3 g_color;

in vec3 g_pos;
in vec3 g_modelPos;
in vec3 g_normal;
in vec2 g_uv;
in vec3 g_dir;
in vec3 g_origin;
in flat int g_id;




layout (binding = 0) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;

}u_camera;

layout (binding = 1) uniform Scene
{
    vec3 ambientColor;
    float ambientIntensity;
    vec4 lightPos;
    vec3 lightColor;
    float lightIntensity;

    float shadowBias;
    float pcfKernelSize;
    float castShadow;

    float kernelRadius;

    mat4 lightViewProj;
}u_scene;

out vec4 fragColor;

void main() {

   

    fragColor = vec4(0.0);

}