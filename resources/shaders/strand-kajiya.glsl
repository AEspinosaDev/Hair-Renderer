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

out vec3 v_lightPos;
out vec3 v_color;
out vec3 v_tangent;

void main() {

    gl_Position = u_camera.viewProj * u_model * vec4(position, 1.0);

    v_tangent = normalize(mat3(transpose(inverse(u_camera.view*u_model))) * tangent);
    v_color = color;
    v_lightPos = (u_camera.view * vec4(u_lightPos,1.0)).xyz;

}

#shader geometry
#version 460 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 v_color[];
in vec3 v_lightPos[];
in vec3 v_tangent[];

out vec3 g_pos;
out vec3 g_normal;
out vec3 g_dir;
out vec3 g_color;
out vec3 g_lightPos;

uniform float u_thickness;

void emitQuadPoint(vec4 origin, vec4 offsetDirection ,float offset,vec3 strandDirection, int id  ){
  
        gl_Position  = origin + offsetDirection * offset;
        g_pos = gl_Position.xyz;
        g_color = v_color[id];
        g_lightPos = v_lightPos[id];
        g_normal = normalize(cross(offsetDirection.xyz,strandDirection.xyz));
        g_dir = v_tangent[id];
        EmitVertex();
}

void main() {
  
        vec4 startPoint = gl_in[0].gl_Position;
        vec4 endPoint = gl_in[1].gl_Position;

        vec4 view0 = -startPoint;
        vec4 view1 = -endPoint;

        vec3 dir0 = v_tangent[0];
        vec3 dir1 = v_tangent[1];

        vec4 right0 = normalize(vec4(cross(dir0.xyz,view0.xyz),0.0));
        vec4 right1 = normalize(vec4(cross(dir1.xyz,view1.xyz),0.0));

        float halfLength = u_thickness*0.5f;

        emitQuadPoint(startPoint,right0,halfLength,dir0,0);
        emitQuadPoint(endPoint,right1,halfLength,dir1,1);
        emitQuadPoint(startPoint,-right0,halfLength,dir0,0);
        emitQuadPoint(endPoint,-right1,halfLength,dir1,1);

}

#shader fragment
#version 460 core

in vec3 g_color;
in flat vec3 g_lightPos;

in vec3 g_pos;
in vec3 g_normal;
in vec3 g_dir;

// ---Hair--
uniform vec3 u_albedo;
uniform vec3 u_spec1;
uniform vec3 u_spec2;
uniform float u_specPwr1;
uniform float u_specPwr2;


out vec4 FragColor;


vec3 shiftTangent(vec3 T, vec3 N, float shift){

  vec3 shiftedT = T+shift*N;
  return normalize(shiftedT);
}
float strandSpecular(vec3 T, vec3 V, vec3 L, float exponent){
  vec3 H = normalize(L + V);
  float u =dot(T,L); //Lambertian
  float t =dot(T,V);
  float dotTH =dot(T,H); //Spec
  float sinTH = sin(acos(dotTH));

  float dirAtten = smoothstep(-1.0, 0.0,
    dotTH);
   
  //return pow(u*t+sin(acos(u))*sin(acos(t)),exponent);
  return dirAtten * pow(sinTH, exponent);
}


//Scheuermann / Kajiya. Kay
vec3 computeLighting(){
  vec3 L = normalize(g_lightPos- g_pos);
  vec3 V = normalize(-g_pos);
  vec3 D = normalize(g_dir);
//   vec3 halfVector = normalize(lightDir + viewDir);

  
//   float shift = uHasTiltText ? texture(uTiltText,_uv).r : 0.0;
  float shift = 0.0;


  // vec3 t1 = shiftTangent(T, N, 0.0 + shift);
  // vec3 t2 = shiftTangent(T,  N, 0.0 + shift);

  vec3 ambient = 0.2*u_albedo;
  // vec3 diffuse = u_albedo*clamp(dot(N,L)*GeometrySmith(N,V,L,0.0),0.0,1.0);
  // vec3 diffuse = u_albedo*clamp(dot(N,L),0.0,1.0);
  vec3 diffuse = u_albedo;

  
  vec3 specular = clamp(u_spec1 * strandSpecular(D, V,L, u_specPwr1),0.0,0.3);
  //vec3 specular = vec3(0.0);
    
//   float highlight = uHasHighlightText ? texture(uHighlightText,_uv).r:1.0;
//   specular += clamp(u_spec2*highlight* strandSpecular(t2,V,L,uSpecularPower2),0.0,1.0);
    float lightIntensity = 1.0;
  return ambient+(diffuse+clamp(specular,0.0,1.0))*lightIntensity;//Include lambertian with different 

}


void main() {
    FragColor = vec4(computeLighting(), 1.0);
    // FragColor = vec4(g_color, 1.0);

}