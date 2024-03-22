#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 uv;
layout(location = 4) in vec3 color;

layout (std140) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;
}u_camera;

uniform mat4 u_model;
uniform vec3 u_lightPos;

out vec3 _pos;
out vec3 _normal;
out vec3 _lightPos;
out vec3 _color;

void main() {
    
    _pos = (u_camera.modelView * vec4(position, 1.0)).xyz;

    _normal = normalize(mat3(transpose(inverse(u_camera.modelView))) * normal);
    _lightPos = (u_camera.view * vec4(u_lightPos, 1.0)).xyz;
    _color = color;

    gl_Position = u_camera.viewProj  * u_model * vec4(position, 1.0);


}

#shader fragment
#version 460 core

in vec3 _pos;
in vec3 _normal;
in vec3 _lightPos;

uniform vec3 u_skinColor;

out vec4 FragColor;

//Surface global properties
vec3 g_normal = _normal;
vec3 g_albedo = u_skinColor;
float g_opacity = 1.0;

// float computeAttenuation() {
    
//     float d = length(i_lightPos - i_pos);
//     float influence = 10.0;
//     float window = pow(max(1 - pow(d / influence, 2), 0), 2);

//     return pow(10 / max(d, 0.0001), 2) * window;
// }

vec3 phong() {

    vec3 lightDir = normalize(_lightPos - _pos);
    vec3 viewDir = normalize(-_pos);
    vec3 halfVector = normalize(lightDir + viewDir);

    vec3 diffuse = clamp(dot(lightDir, g_normal), 0.0, 1.0) * vec3(1.0,1.0,1.0);

    //Blinn specular term
    vec3 specular = pow(max(dot(g_normal, halfVector), 0.0), 20.0) * 5.0 * vec3(1.0,1.0,1.0);

    vec3 color =  g_albedo; //surface

    // float att = computeAttenuation();
     float att = 1.0;
    vec3 ambient = vec3(1.0f)*0.2f;


    return (ambient +diffuse + specular) * color * att * 1.0;

}

void main() {
    // FragColor = vec4(vec3(1.0,0.0,0.0), 1.0);
    FragColor = vec4(phong(), g_opacity);

}