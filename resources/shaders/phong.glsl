#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 uv;
layout(location = 4) in vec3 color;

uniform mat4 u_viewProj;
uniform mat4 u_modelView;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 _pos;
out vec3 _normal;
out vec3 _lightPos;

void main() {
    _pos = (u_modelView * vec4(position, 1.0)).xyz;
    _normal = normalize(mat3(transpose(inverse(u_modelView))) * normal);
    _lightPos = (u_view * vec4(vec3(5.0,3.0,-3.0), 1.0)).xyz;

    gl_Position = u_viewProj  * u_model * vec4(position, 1.0);

}

#shader fragment
#version 460 core

in vec3 _pos;
in vec3 _normal;
in vec3 _lightPos;


out vec4 FragColor;

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

    vec3 diffuse = clamp(dot(lightDir, _normal), 0.0, 1.0) * vec3(1.0,1.0,1.0);

    //Blinn specular term
    vec3 specular = pow(max(dot(_normal, halfVector), 0.0), 20.0) * 5.0 * vec3(1.0,1.0,1.0);

    vec3 color =  vec3(1.0,0.0,0.0); //surface

    // float att = computeAttenuation();
     float att = 1.0;


    return (diffuse + specular) * color * att * 1.0;

}

void main() {
    FragColor = vec4(phong(), 1.0);
}