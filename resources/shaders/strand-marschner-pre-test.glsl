#stage vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 uv;
layout(location = 4) in vec3 color;


layout (binding = 0) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;
    vec3 position;
    float exposure;

}u_camera;

uniform mat4 u_model;

out vec3 v_color;
out vec3 v_dir;
out int v_id;
out vec3 v_pos;

void main() {

    gl_Position =  u_camera.viewProj * u_model * vec4(position, 1.0);

    v_dir = normalize(mat3(transpose(inverse(u_model))) * tangent);
    v_dir = tangent;
    v_color = color;
    v_id = gl_VertexID;
    v_pos = (u_model * vec4(position, 1.0)).xyz;

}



#stage fragment
#version 460 core


in vec3 v_color;

in vec3 v_pos;
// in vec3 g_normal;
in vec3 v_dir;
in flat int v_id;




layout (binding = 0) uniform Camera
{
    mat4 viewProj;
    mat4 modelView;
    mat4 view;
    vec3 position;
    float exposure;

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

    vec4 frustrumData;
}u_scene;

// ---Hair--
struct HairMaterial{
    vec3 baseColor;
    float specular;
    float roughness;
    float scatter;
    float shift;
    float ior;

    bool glints; 
    bool useScatter;
    bool coloredScatter;

    bool r;
    bool tt;
    bool trt;

    bool occlusion;
};

uniform float u_thickness;
uniform HairMaterial u_hair;
uniform sampler2D u_shadowMap;
uniform sampler2D u_noiseMap;
uniform sampler2D u_depthMap;

uniform samplerCube u_irradianceMap;
uniform bool u_useSkybox;

uniform sampler2D u_m;
uniform sampler2D u_n;

uniform vec3 u_BVCenter;

float scatterWeight = 0.0;

//Constant
const float PI = 3.14159265359;

out vec4 fragColor;

vec3 shiftTangent(vec3 T, vec3 N, float shift){
  vec3 shiftedT = T+shift*N;
  return normalize(shiftedT);
}

float linearizeDepth(float depth, float near, float far) {
    float z = depth * 2.0 - 1.0; 
    float linearZ = (2.0 * near * far) / (far + near - z * (far - near));
    return (linearZ - near) / (far - near);
}

//Real-time Marschnerr
vec3 computeLighting(float beta, float shift, vec3 radiance, bool r, bool tt, bool trt){

  //--->>>View space
  vec3 wi = normalize(u_scene.lightPos.xyz- v_pos);    //Light vector
//   vec3 wi = normalize(vec3(0.0,0.0,-10.0)-v_pos);    //Light vectoSr
  // vec3 n  = g_normal;                                  //Strand shading normal
//   vec3 v  = normalize(vec3(2.0,2.0,-10.0)-v_pos);                         //Camera vector
  vec3 v  = normalize(u_camera.position-v_pos);                         //Camera vector
  vec3 u  = normalize(v_dir);                          //Strand tangent/direction


  //Theta & Phi
  float sin_thI = dot(u,wi);
  float sin_thR  = dot(u,v);
  vec3 wiPerp    = wi - sin_thI * u;
  vec3 vPerp     = v - sin_thR * u;
  float cos_phiD = dot(vPerp,wiPerp)*inversesqrt(dot(wiPerp,wiPerp)*dot(vPerp,vPerp));
  float cos_thD    = cos((asin(sin_thI)-asin(sin_thR))/2.0);

  //LUTs
  //Marschner M
  vec2 uvM = vec2(sin_thI,sin_thR)*0.5+0.5;
  uvM.y = 1.0-uvM.y;
  vec4 mM  = texture(u_m,uvM).rgba; 
  // float cos_thD = mM.a; 
  //Marschner N
  vec2 uvN = vec2(cos_phiD,cos_thD)*0.5+0.5;
  uvN.y = 1.0-uvN.y;
  vec4 mN  = texture(u_n,uvN).rgba;

  float R   = r ?   mM.r * mN.r : 0.0; 
  float TT  = tt ?  mM.g * mN.g : 0.0; 
  float TRT = trt ? mM.b * mN.b: 0.0; 

  vec3 absColor = u_hair.baseColor;

  vec3 specular = vec3((R+TT+TRT)/max(0.0,cos_thD*cos_thD));
  specular*= radiance;

  vec3 diffuse  = u_hair.baseColor;

  // return (specular+diffuse) * radiance;
  return specular;

}




void main() {

    vec3 color  = computeLighting(
      u_hair.roughness,
      u_hair.shift,
      u_scene.lightColor*u_scene.lightIntensity,
      u_hair.r,
      u_hair.tt,
      u_hair.trt);

    
    // color+=ambient;

   
    // color = color / (color + vec3(1.0));
    //   const float GAMMA = 2.2;
    //   color = pow(color, vec3(1.0 / GAMMA));

    fragColor = vec4(color,1.0f);

}