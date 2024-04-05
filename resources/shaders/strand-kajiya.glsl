#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 uv;
layout(location = 4) in vec3 color;


uniform mat4 u_model;

out vec3 v_color;
out vec3 v_tangent;
out vec3 v_rawTangent;


void main() {

    gl_Position =  u_model * vec4(position, 1.0);

    v_tangent = normalize(mat3(transpose(inverse(u_model))) * tangent);
    v_rawTangent = tangent;

    v_color = color;

}

#shader geometry
#version 460 core

// #define NORMAL_MAPPING

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 v_color[];
in vec3 v_tangent[];
in vec3 v_rawTangent[];

layout (binding = 0) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;

}u_camera;

out vec3 g_pos;
out vec3 g_normal;
out vec2 g_uv;
out vec3 g_dir;
out vec3 g_color;
out vec3 g_origin;
#ifdef NORMAL_MAPPING
out mat3 g_TBN;
#endif

uniform float u_thickness;
uniform vec3 u_camPos;
uniform mat4 u_model;

void emitQuadPoint(vec4 origin, 
                  vec4 right,
                  float offset,
                  vec3 forward, 
                  vec3 normal, 
                  vec2 uv,
                  int id){
  
        gl_Position =  u_camera.viewProj *(origin + right * offset);
        g_dir = normalize(mat3(transpose(inverse(u_camera.view*u_model))) * v_rawTangent[id]);
        g_color = v_color[id];
        g_pos = gl_Position.xyz;
        g_uv = uv;
        g_normal =  normalize(mat3(transpose(inverse(u_camera.view*u_model))) * normal);
        g_origin = (u_camera.viewProj * origin).xyz; 

        //In case of normal mapping
#ifdef NORMAL_MAPPING
        g_TBN = mat3(right.xyz, forward, g_normal);
#endif

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

        emitQuadPoint(startPoint,right0,halfLength,dir0,normal0,vec2(1.0,0.0),0);
        emitQuadPoint(endPoint,right1,halfLength,dir1,normal1,vec2(1.0,1.0),1);
        emitQuadPoint(startPoint,-right0,halfLength,dir0,normal0,vec2(-1.0,0.0),0);
        emitQuadPoint(endPoint,-right1,halfLength,dir1,normal1,vec2(-1.0,1.0),1);

}

#shader fragment
#version 460 core

// #define NORMAL_MAPPING

in vec3 g_color;

in vec3 g_pos;
in vec3 g_normal;
in vec2 g_uv;
in vec3 g_dir;
in vec3 g_origin;
#ifdef NORMAL_MAPPING
in mat3 g_TBN;
#endif

layout (binding = 1) uniform Scene
{
    vec4 ambient;
    vec4 lightPos;
    vec4 lightColor;
}u_scene;

// ---Hair--
uniform vec3 u_albedo;
uniform vec3 u_spec1;
uniform vec3 u_spec2;
uniform float u_specPwr1;
uniform float u_specPwr2;
uniform float u_thickness;
#ifdef NORMAL_MAPPING
uniform sampler2D u_normalText;
#endif


out vec4 FragColor;

vec3 sh_normal;
float halfLength = u_thickness*0.5f;



float computePointInCircleSurface(float u,float radius) {
    return sqrt(1.0 - u * u);
}

void computeNormal(){
    float offsetMag = computePointInCircleSurface(g_uv.x,halfLength);
    vec3 offsetPos = g_pos + g_normal * (halfLength/offsetMag);
    
    sh_normal = normalize(offsetPos-g_origin);
    sh_normal =vec3(offsetMag);

}
#ifdef NORMAL_MAPPING
void computeNormalFromTexture(){
    vec3 textNormal = texture(u_normalText, g_uv).rgb;
    textNormal = textNormal * 2.0 - 1.0;   
    sh_normal = normalize(g_TBN * textNormal); 
    
}
#endif

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
  vec3 L = normalize(u_scene.lightPos.xyz- g_pos);
  vec3 V = normalize(-g_pos);
  vec3 D = normalize(g_dir);
vec3 halfVector = normalize(L + V);

  
//   float shift = uHasTiltText ? texture(uTiltText,_uv).r : 0.0;
  float shift = 0.0;


  // vec3 t1 = shiftTangent(T, N, 0.0 + shift);
  // vec3 t2 = shiftTangent(T,  N, 0.0 + shift);

  vec3 ambient = u_albedo;
  // vec3 diffuse = u_albedo*clamp(dot(sh_normal,L),0.0,1.0);
  // // vec3 diffuse = u_albedo;

  
  // vec3 specular = clamp(u_spec1 * strandSpecular(D, V,L, u_specPwr1),0.0,0.3);
  //vec3 specular = vec3(0.0);

  vec3 diffuse = clamp(dot(L, g_normal), 0.0, 1.0) * vec3(1.0,1.0,1.0);
  vec3 specular = pow(max(dot(g_normal, halfVector), 0.0), 20.0) * 5.0 * vec3(1.0,1.0,1.0);
  vec3 color =  u_albedo; //surface
  return (ambient +diffuse ) * color  * 1.0;
    
//   float highlight = uHasHighlightText ? texture(uHighlightText,_uv).r:1.0;
//   specular += clamp(u_spec2*highlight* strandSpecular(t2,V,L,uSpecularPower2),0.0,1.0);
  return ambient+((u_albedo+specular)*clamp(dot(sh_normal,L),0.0,1.0))*u_scene.lightColor.w;//Include lambertian with different 

}


void main() {

    computeNormal();
    FragColor = vec4(computeLighting(), 1.0);

    FragColor = vec4(sh_normal*u_albedo, 1.0);

}