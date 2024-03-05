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
out vec3 v_tangent;

void main() {
    gl_Position = u_viewProj * u_model * vec4(position, 1.0);

    v_tangent = normalize(mat3(transpose(inverse(u_view*u_model))) * tangent);
    // v_tangent = tangent;
    v_color = color;
    v_lightPos = (u_view * vec4(u_lightPos,1.0)).xyz;
}

#shader geometry
#version 460 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 v_color[];
in vec3 v_lightPos[];
in vec3 v_tangent[];

out vec3 g_color;
out vec3 g_lightPos;

out vec3 g_pos;
out vec3 g_normal;
out vec3 g_tangent;

uniform float u_thickness;

void emitQuadPoint(vec4 origin, vec4 offsetDirection ,float offset,vec3 strandDirection, int id  ){
        gl_Position  = origin + offsetDirection * offset;
        g_pos = gl_Position.xyz;
        g_color = v_color[id];
        g_lightPos = v_lightPos[id];
        g_normal = normalize(cross(offsetDirection.xyz,strandDirection.xyz));
        // g_tangent = strandDirection.xyz;
        g_tangent = v_tangent[0];
        EmitVertex();
}

void main() {
        vec4 startPoint = gl_in[0].gl_Position;
        vec4 endPoint = gl_in[1].gl_Position;

        // vec4 strandDirection = normalize(endPoint - startPoint);
        vec3 strandDirection = v_tangent[0];
        vec3 strandDirectionL = v_tangent[1];



        vec4 startViewDir = -startPoint;
        vec4 endViewDir = -endPoint;

        // vec3 interpolatedDirection = mix(v_tangent[0],v_tangent[1],0.5)

        vec3 crossStartNormal =normalize(cross(strandDirection.xyz,startViewDir.xyz));
        vec3 crossEndNormal = normalize(cross(strandDirectionL.xyz,endViewDir.xyz));
        vec4 startNormal = vec4(crossStartNormal,0.0);
        vec4 endNormal = vec4(crossEndNormal,0.0);

        float halfLength = u_thickness*0.5f;

        emitQuadPoint(startPoint,startNormal,halfLength,strandDirection,0);
        emitQuadPoint(endPoint,endNormal,halfLength,strandDirectionL,1);
        emitQuadPoint(startPoint,-startNormal,halfLength,strandDirection,0);
        emitQuadPoint(endPoint,-endNormal,halfLength,strandDirectionL,1);

}

#shader fragment
#version 460 core

in vec3 g_color;
in flat vec3 g_lightPos;

in vec3 g_pos;
in vec3 g_normal;
in vec3 g_tangent;

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
//   vec3 L = normalize(g_lightPos - _pos);
//   vec3 V = normalize(-_pos);
//   vec3 T =uHasDirectionText ? normalize(_TBN * (texture(uDirectionText,_uv).rbg * 2.0) - 1.0): normalize(_bitangent);
//   vec3 N = normalize(_normal);

  vec3 L = normalize(g_lightPos- g_pos);
  vec3 V = normalize(-g_pos);
  vec3 T = normalize(g_tangent);
//   vec3 halfVector = normalize(lightDir + viewDir);

  
  vec3 Ks = vec3(1.0,1.0,1.0);
  vec3 Ka = g_color;
  vec3 Kd = g_color;
//   float shift = uHasTiltText ? texture(uTiltText,_uv).r : 0.0;
  float shift = 0.0;


  // vec3 t1 = shiftTangent(T, N, 0.0 + shift);
  // vec3 t2 = shiftTangent(T,  N, 0.0 + shift);

  vec3 ambient = 0.2*Ka;
  // vec3 diffuse = Kd*clamp(dot(N,L)*GeometrySmith(N,V,L,0.0),0.0,1.0);
  // vec3 diffuse = Kd*clamp(dot(N,L),0.0,1.0);
  vec3 diffuse = vec3(0.0);

  
  vec3 specular = clamp(Ks * strandSpecular(T, V,L, 120.0),0.0,0.3);
  //vec3 specular = vec3(0.0);
    
//   float highlight = uHasHighlightText ? texture(uHighlightText,_uv).r:1.0;
//   specular += clamp(Ks*highlight* strandSpecular(t2,V,L,uSpecularPower2),0.0,1.0);
    float lightIntensity = 1.0;
  return ambient+(diffuse+clamp(specular,0.0,1.0))*lightIntensity;//Include lambertian with different 

}


void main() {
    // FragColor = vec4(computeLighting(), 1.0);
    FragColor = vec4(g_color, 1.0);

}